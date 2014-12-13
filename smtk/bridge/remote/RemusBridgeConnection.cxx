//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef SHIBOKEN_SKIP
#include "smtk/bridge/remote/RemusBridgeConnection.h"
#include "smtk/bridge/remote/RemusRemoteBridge.h"

#include "smtk/io/ImportJSON.h"
#include "smtk/io/ExportJSON.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/common/Paths.h"
#include "smtk/common/Environment.h"

#include "smtk/Function.h"

#include <functional>

#include "remus/client/ServerConnection.h"

#include "remus/server/Server.h"
#include "remus/server/WorkerFactory.h"

#include "remus/proto/Job.h"
#include "remus/proto/JobContent.h"
#include "remus/proto/JobSubmission.h"

#include "cJSON.h"

using smtk::common::UUID;
using namespace smtk::common;
using namespace smtk::model;
using namespace smtk::io;
using namespace smtk::placeholders;

namespace smtk {
  namespace bridge {
    namespace remote {

RemusBridgeConnection::RemusBridgeConnection()
{
  Paths paths;
  StringList spaths = paths.workerSearchPaths();
  this->m_searchDirs = searchdir_t(spaths.begin(), spaths.end());
  this->m_modelMgr = smtk::model::Manager::create();
}

RemusBridgeConnection::~RemusBridgeConnection()
{
}

/**\brief Indicate location where Remus worker files may live.
  *
  * Before calling connectToServer(void), call this method
  * for each directory you would like the process-local Remus
  * server to search for worker files.
  * This has no effect once connectToServer() has been invoked.
  */
void RemusBridgeConnection::addSearchDir(const std::string& searchDir)
{
  this->m_searchDirs.insert(searchDir);
}

/**\brief Reset all of the search directories added with addSearchDir().
  *
  * If \a clearDefaultsToo is true, then the pre-existing search
  * paths provided by smtk::common::Paths::workerSearchPaths()
  * are eliminated as well.
  */
void RemusBridgeConnection::clearSearchDirs(bool clearDefaultsToo)
{
  this->m_searchDirs.clear();
  if (!clearDefaultsToo)
    {
    Paths paths;
    StringList spaths = paths.workerSearchPaths();
    this->m_searchDirs = searchdir_t(spaths.begin(), spaths.end());
    }
}

/**\brief Initiate the connection to the Remus server.
  *
  * This constructs a Remus server connection; it does
  * not necessarily verify that communication to/from
  * the server works.
  */
bool RemusBridgeConnection::connectToServer(const std::string& hostname, int port)
{
  // TODO: Drop any current connection and reset bridges? Copy-on-connect? ???
  if (hostname.empty() || hostname == "local")
    {
    // Start a process-local server
    boost::shared_ptr<remus::server::WorkerFactory> factory(new remus::server::WorkerFactory());
    int maxWorkers = 5;
    std::string env = smtk::common::Environment::getVariable("SMTK_REMUS_MAX_WORKERS");
    if (!env.empty())
      {
      std::stringstream envVal(env);
      envVal >> maxWorkers;
      }
    factory->setMaxWorkerCount(maxWorkers);

    // Add search directories, if any.
    for (searchdir_t::const_iterator it = this->m_searchDirs.begin(); it != this->m_searchDirs.end(); ++it)
      factory->addWorkerSearchDirectory(*it);

    this->m_localServer = smtk::shared_ptr<remus::Server>(
      new remus::Server(remus::server::ServerPorts(),factory));
    this->m_localServer->startBrokering();

    // Connect to the process-local server
    this->m_conn = remus::client::ServerConnection();
    }
  else
    this->m_conn = remus::client::ServerConnection(hostname, port);
  this->m_client =
    smtk::shared_ptr<remus::client::Client>(
      new remus::client::Client(this->m_conn));
  this->m_remoteBridgeNameToType.clear();
  return true;
}

/// Return the list of bridges available on the server (not the local modelManager()'s list).
std::vector<std::string> RemusBridgeConnection::bridgeNames()
{
  std::vector<std::string> resultVec;
  if (this->m_remoteBridgeNameToType.empty())
    {
    if (!this->m_client)
      this->connectToServer();
    remus::common::MeshIOTypeSet mtypes = this->m_client->supportedIOTypes();
    remus::common::MeshIOTypeSet::const_iterator mit;
    for (mit = mtypes.begin(); mit != mtypes.end(); ++mit)
      {
      if (mit->outputType() == "smtk::model[native]") // TODO: Eliminate this magic string?
        {
        // Obtain the solid-modeling kernel "requirements", including the file types and operators.
        remus::proto::JobRequirementsSet kernelInfos = this->m_client->retrieveRequirements(*mit);
        remus::proto::JobRequirementsSet::const_iterator kit;
        for (kit = kernelInfos.begin(); kit != kernelInfos.end(); ++kit)
          {
          const remus::proto::JobRequirements& kernelInfo(*kit);
          RemusStaticBridgeInfo binfo =
            RemusRemoteBridge::createFunctor(
              shared_from_this(), kernelInfo, mit->inputType());
          // FIXME: If we implemented it, we could pass a method to
          //        accept remotely-provided pre-construction setup
          //        options to the bridge. But that is too fancy for now.
          smtk::model::BridgeRegistrar::registerBridge(
            binfo.name(), binfo.tags(),
            smtk::bind(&RemusStaticBridgeInfo::staticSetup, binfo, _1, _2),
            binfo);
          this->m_remoteBridgeNameToType[binfo.name()] = mit->inputType();
          smtkInfoMacro(log(), "Added model worker named \""
            << binfo.name() << "\", type \"" << mit->inputType() << "\".");
          }
        }
      }
    }
  std::map<std::string,std::string>::const_iterator bit;
  for (bit = this->m_remoteBridgeNameToType.begin(); bit != this->m_remoteBridgeNameToType.end(); ++bit)
    resultVec.push_back(bit->first);

  return resultVec;
}

/**\brief Start a worker of a particular type, or revive a particular session ID.
  *
  */
int RemusBridgeConnection::staticSetup(
  const std::string& bridgeName,
  const std::string& optName,
  const smtk::model::StringList& optVal)
{
  (void) this->bridgeNames(); // ensure that we've fetched the bridge names from the server.
  if (this->m_remoteBridgeNameToType.find(bridgeName) == this->m_remoteBridgeNameToType.end())
    return 0;

  remus::proto::JobRequirements jreq;
  if (!this->findRequirementsForRemusType(jreq, bridgeName))
    return 0;

  //FIXME: Sanitize bridgeName!
  cJSON* params;
  cJSON* request = ExportJSON::createRPCRequest("bridge-setup", params, /*id*/ "1");
  cJSON_AddItemToObject(params, "bridge-name", cJSON_CreateString(bridgeName.c_str()));
  cJSON_AddItemToObject(params, "option-name", cJSON_CreateString(optName.c_str()));
  cJSON_AddItemToObject(params, "option-value", ExportJSON::createStringArray(optVal));
  cJSON* result = this->jsonRPCRequest(request, jreq);
  cJSON* rval;
  if (
    // Was JSON parsable?
    !result ||
    // Is the result an Object (as required by JSON-RPC 2.0)?
    result->type != cJSON_Object ||
    // Does the result Object have a field named "result" (req'd by JSON-RPC)?
    !(rval = cJSON_GetObjectItem(result, "result")) ||
    // Is the "result" field an Object with a child that is also an object?
    rval->type != cJSON_Number)
    {
    // TODO: See if result has "error" key and report it.
    if (result)
      cJSON_Delete(result);
    return 0;
    }
  return rval->valueint;
}

/**\brief Start a worker of a particular type, or revive a particular session ID.
  *
  */
UUID RemusBridgeConnection::beginBridgeSession(const std::string& bridgeName)
{
  (void) this->bridgeNames(); // ensure that we've fetched the bridge names from the server.
  if (this->m_remoteBridgeNameToType.find(bridgeName) == this->m_remoteBridgeNameToType.end())
    return UUID::null();

  remus::proto::JobRequirements jreq;
  if (!this->findRequirementsForRemusType(jreq, bridgeName))
    return UUID::null();

  cJSON* params;
  cJSON* req = ExportJSON::createRPCRequest("create-bridge", params, /*id*/ "1", cJSON_Object);
  cJSON_AddItemToObject(
    params, "bridge-name",
    cJSON_CreateString(bridgeName.c_str())); //jreq.meshTypes().inputType().c_str()));

  cJSON* result = this->jsonRPCRequest(req, jreq);

  cJSON* bridgeObj;
  cJSON* bridgeIdObj;
  cJSON* opsObj;
  cJSON* nameObj;
  if (
    // Was JSON parsable?
    !result ||
    // Is the result an Object (as required by JSON-RPC 2.0)?
    result->type != cJSON_Object ||
    // Does the result Object have a field named "result" (req'd by JSON-RPC)?
    !(bridgeObj = cJSON_GetObjectItem(result, "result")) ||
    // Is the "result" field an Object with a child that is also an object?
    bridgeObj->type != cJSON_Object ||
    !(bridgeIdObj = bridgeObj->child) ||
    bridgeIdObj->type != cJSON_Object ||
    // Does the first child have a valid name? (This is the bridge session ID)
    !bridgeIdObj->string ||
    !bridgeIdObj->string[0] ||
    // Does the first child have fields "name" and "ops" of type String and Array?
    !(nameObj = cJSON_GetObjectItem(bridgeIdObj, "name")) ||
    nameObj->type != cJSON_String ||
    !nameObj->valuestring ||
    !nameObj->valuestring[0] ||
    !(opsObj = cJSON_GetObjectItem(bridgeIdObj, "ops")) ||
    opsObj->type != cJSON_String
    )
      {
      // TODO: See if result has "error" key and report it.
      if (result)
        cJSON_Delete(result);
      return UUID::null();
      }

  // OK, construct a special "forwarding" bridge locally.
  RemusRemoteBridge::Ptr bridge = RemusRemoteBridge::create();
  bridge->setup(this, jreq);
  // The ImportJSON registers this bridge with the model manager.
  if (smtk::io::ImportJSON::ofRemoteBridgeSession(
      bridgeIdObj, bridge, this->m_modelMgr))
    {
    // Failure.
    }
  //this->m_modelMgr->registerBridgeSession(bridge);

  cJSON_Delete(result);

  UUID bridgeId = bridge->sessionId();
  this->m_remoteBridgeSessionIds[bridgeId] = bridgeName;
  /*
  smtkInfoMacro(log(),
    << "Updating worker \"" << jreq.workerName() << "\""
    << " tag \"" << jreq.tag() << "\".");
  jreq.tag(bridgeId.toString());
  */
  bridge->setup(this, jreq);
  return bridgeId;
}

/**\brief Shut down the worker with the given \a bridgeSessionId.
  *
  */
bool RemusBridgeConnection::endBridgeSession(const UUID& bridgeSessionId)
{
  std::map<UUID,std::string>::iterator it =
    this->m_remoteBridgeSessionIds.find(bridgeSessionId);
  if (it == this->m_remoteBridgeSessionIds.end())
    return false;

  // Unhook our local cmbForwardingBridge representing the remote.
  smtk::model::Bridge::Ptr bridgeBase = this->m_modelMgr->findBridgeSession(bridgeSessionId);
  RemusRemoteBridge::Ptr bridge = smtk::dynamic_pointer_cast<RemusRemoteBridge>(bridgeBase);
  if (bridge)
    {
    this->m_modelMgr->unregisterBridgeSession(bridge);

    // Tell the server to unregister this session.
    // (Since the server's model manager should hold the only shared pointer
    // to the session, this should kill it.)
    std::string note =
      "{\"jsonrpc\":\"2.0\", \"method\":\"delete-bridge\", \"params\":{\"session-id\":\"" +
      bridgeSessionId.toString() + "\"}}";
    this->jsonRPCNotification(note, bridge->remusRequirements());
    }

  // Now remove the entry from the proxy's list of sessions.
  this->m_remoteBridgeSessionIds.erase(it);
  return true;
}

/**\brief Obtain a RemusRemoteBridge given its session ID.
  *
  */
RemusRemoteBridge::Ptr RemusBridgeConnection::findBridgeSession(
  const smtk::common::UUID& bridgeSessionId)
{
  Bridge::Ptr sess = this->m_modelMgr->findBridgeSession(bridgeSessionId);
  return smtk::dynamic_pointer_cast<RemusRemoteBridge>(sess);
}

/**\brief Return a list of file types supported by a particular bridge.
  */
std::vector<std::string> RemusBridgeConnection::supportedFileTypes(
  const std::string& bridgeName)
{
  std::vector<std::string> resultVec;
  cJSON* params;
  cJSON* request = ExportJSON::createRPCRequest("bridge-filetypes", params, /*id*/ "1", cJSON_Object);
  cJSON_AddItemToObject(params, "bridge-name", cJSON_CreateString(bridgeName.c_str()));

  // Now we need a worker to contact. If one already exists of the
  // given type, ask it. Otherwise, find a "blank" session that the
  // server advertises.
  // I. Look for an open session of the required type.
  RemusRemoteBridge::Ptr bridge = this->findBridgeForRemusType(bridgeName);
  remus::proto::JobRequirements jreq;
  if (bridge)
    {
    jreq = bridge->remusRequirements();
    }
  else if (!this->findRequirementsForRemusType(jreq, bridgeName))
    {
    return resultVec;
    }
  cJSON* result = this->jsonRPCRequest(request, jreq);
  cJSON* sarr;
  if (
    !result ||
    result->type != cJSON_Object ||
    !(sarr = cJSON_GetObjectItem(result, "result")) ||
    sarr->type != cJSON_Array)
    {
    smtkErrorMacro(this->log(),
      "Invalid filetype response \""
      << (result ? cJSON_Print(result) : "null") << "\"");
    // TODO: See if result has "error" key and report it.
    if (result)
      cJSON_Delete(result);
    return std::vector<std::string>();
    }

  smtk::io::ImportJSON::getStringArrayFromJSON(sarr, resultVec);
  cJSON_Delete(result);
  return resultVec;
}

/**\brief Read a file without requiring a pre-existing bridge session.
  *
  * Some bridges (such as CGM) need a fileType to be specified.
  * You may leave it empty and hope the kernel will do its best
  * to infer it.
  */
smtk::model::OperatorResult RemusBridgeConnection::readFile(
  const std::string& fileName,
  const std::string& fileType,
  const std::string& bridgeName)
{
  std::string actualBridgeName;
  if (bridgeName.empty())
    {
    (void) this->bridgeNames(); // ensure that we've fetched the bridge names from the server.
    std::map<std::string,std::string>::const_iterator bnit;
    for (
      bnit = this->m_remoteBridgeNameToType.begin();
      bnit != this->m_remoteBridgeNameToType.end() && actualBridgeName.empty();
      ++bnit)
      {
      std::vector<std::string> fileTypesForBridge = this->supportedFileTypes(bnit->first);
      std::vector<std::string>::const_iterator fit;
      for (fit = fileTypesForBridge.begin(); fit != fileTypesForBridge.end(); ++fit)
        {
        std::string::size_type fEnd;
        std::string::size_type eEnd = fit->find(' ');
        std::string ext(*fit, 0, eEnd);
        smtkInfoMacro(log(), "Looking for \"" << ext << "\".");
        if ((fEnd = fileName.rfind(ext)) && (fileName.size() - fEnd == eEnd))
          { // matching substring is indeed at end of fileName
          actualBridgeName = bnit->first;
          smtkInfoMacro(log(), "Found bridge type " << actualBridgeName << " for " << fileName << ".");
          break;
          }
        }
      }
    }
  else
    {
    actualBridgeName = bridgeName;
    }
  RemusRemoteBridge::Ptr bridge = this->findBridgeForRemusType(actualBridgeName);
  if (!bridge)
    { // No existing bridge of that type. Create a new remote session.
    UUID bridgeId = this->beginBridgeSession(actualBridgeName);
    bridge =
      smtk::dynamic_pointer_cast<RemusRemoteBridge>(
        this->m_modelMgr->findBridgeSession(bridgeId));
    }
  if (!bridge)
    {
    smtkInfoMacro(log(), "Could not find or create bridge of type \"" << actualBridgeName << "\".");
    return smtk::model::OperatorResult();
    }
  smtkInfoMacro(log(), "Found bridge " << bridge->sessionId() << " (" << actualBridgeName << ").");

  smtk::model::OperatorPtr readOp = bridge->op("read");
  if (!readOp)
    {
    smtkInfoMacro(log(), "Could not create read operator for bridge"
      << " \"" << actualBridgeName << "\""
      << " (" << bridge->sessionId() << ").");
    return smtk::model::OperatorResult();
    }

  readOp->specification()->findFile("filename")->setValue(fileName);
  smtk::attribute::StringItem::Ptr fileTypeItem =
    readOp->specification()->findString("filetype");
  if (fileTypeItem)
    {
    fileTypeItem->setValue(fileType);
    }

  smtk::model::OperatorResult result = readOp->operate();

  // Fetch affected models
  smtk::attribute::ModelEntityItem::Ptr models =
    result->findModelEntity("model");
  if (models)
    {
    int numModels = models->numberOfValues();
    for (int i = 0; i < numModels; ++i)
      this->fetchWholeModel(models->value(i).entity());
    }
  this->m_modelMgr->assignDefaultNames();
  return result;
}

/**\brief
  *
  */
std::vector<std::string> RemusBridgeConnection::operatorNames(const std::string& bridgeName)
{
  (void)bridgeName;
  std::vector<std::string> result;
  return result;
}

/**\brief
  *
  */
std::vector<std::string> RemusBridgeConnection::operatorNames(const UUID& bridgeSessionId)
{
  (void)bridgeSessionId;
  std::vector<std::string> result;
  return result;
}

/**\brief
  *
  */
smtk::model::OperatorPtr RemusBridgeConnection::createOperator(
  const UUID& bridgeOrModel, const std::string& opName)
{
  (void)opName;
  (void)bridgeOrModel;
  smtk::model::OperatorPtr empty;
  return empty;
}

/**\brief
  *
  */
smtk::model::OperatorPtr RemusBridgeConnection::createOperator(
  const std::string& bridgeName, const std::string& opName)
{
  (void)opName;
  (void)bridgeName;
  smtk::model::OperatorPtr empty;
  return empty;
}

/**\brief Ask the current worker to return all of the entities at the remote end.
  *
  * Warning: This will overwrite this->m_modelMgr.
  */
void RemusBridgeConnection::fetchWholeModel(const UUID& modelId)
{
  // Find bridge from modelId, then job requirements from bridge.
  smtk::model::Bridge::Ptr bridgeBase = this->m_modelMgr->bridgeForModel(modelId);
  RemusRemoteBridge::Ptr bridge = smtk::dynamic_pointer_cast<RemusRemoteBridge>(bridgeBase);
  if (!bridge)
    return;

  cJSON* params;
  cJSON* request = ExportJSON::createRPCRequest("fetch-model", params, /*id*/ "1", cJSON_Array);
  cJSON* response = this->jsonRPCRequest(request, bridge->remusRequirements());
  cJSON* model;
  cJSON* topo;
  //smtkInfoMacro(log(), " ----- \n\n\n" << cJSON_Print(response) << "\n ----- \n\n.");
  if (
    response &&
    (model = cJSON_GetObjectItem(response, "result")) &&
    model->type == cJSON_Object &&
    (topo = cJSON_GetObjectItem(model, "topo")))
    smtk::io::ImportJSON::ofManager(topo, this->m_modelMgr);
  cJSON_Delete(response);
}

smtk::model::ManagerPtr RemusBridgeConnection::modelManager()
{
  return this->m_modelMgr;
}

void RemusBridgeConnection::setModelManager(smtk::model::ManagerPtr mgr)
{
  this->m_modelMgr = mgr;
}

cJSON* RemusBridgeConnection::jsonRPCRequest(cJSON* req, const remus::proto::JobRequirements& jreq)
{
  char* reqStr = cJSON_Print(req);
  cJSON_Delete(req);
  cJSON* response = this->jsonRPCRequest(reqStr, jreq);
  free(reqStr);
  return response;
}

/// Perform a synchronous JSON-RPC request.
cJSON* RemusBridgeConnection::jsonRPCRequest(const std::string& request, const remus::proto::JobRequirements& jreq)
{
  remus::proto::JobContent jcnt(remus::common::ContentFormat::JSON, request);
  remus::proto::JobSubmission jsub(jreq);
  jsub.insert(
    remus::proto::JobSubmission::value_type(
      "request",
      remus::proto::JobContent(
        remus::common::ContentFormat::JSON,
        request)));
  // Submit the job
  remus::proto::Job jd = this->m_client->submitJob(jsub);

  // Wait for the job to complete.
  remus::proto::JobStatus jobState = this->m_client->jobStatus(jd);
  // TODO: UGLY!
  while (jobState.good())
    jobState = this->m_client->jobStatus(jd);

  remus::proto::JobResult jres = this->m_client->retrieveResults(jd);
  if (!jres.valid() || jres.formatType() != remus::common::ContentFormat::JSON)
    return NULL;
  cJSON* result = cJSON_Parse(jres.data());
  return result;
}

void RemusBridgeConnection::jsonRPCNotification(cJSON* note, const remus::proto::JobRequirements& jreq)
{
  char* noteStr = cJSON_Print(note);
  cJSON_Delete(note);
  this->jsonRPCNotification(noteStr, jreq);
  free(noteStr);
}

void RemusBridgeConnection::jsonRPCNotification(const std::string& note, const remus::proto::JobRequirements& jreq)
{
  remus::proto::JobContent jcnt(remus::common::ContentFormat::JSON, note);
  remus::proto::JobSubmission jsub(jreq);
  jsub.insert(
    remus::proto::JobSubmission::value_type(
      "request",
      remus::proto::JobContent(
        remus::common::ContentFormat::JSON,
        note)));
  // Submit the job
  remus::proto::Job jd = this->m_client->submitJob(jsub);
  (void)jd;
  /*
   * TODO: Should we wait until the server tells us the job was complete?
   *       Probably not, according to JSON-RPC spec.
  remus::proto::JobResult jres = this->m_client->retrieveResults(jd);
  if (!jres.valid() || jres.formatType() != remus::common::ContentFormat::JSON)
    return NULL;
  cJSON* result = cJSON_Parse(jres.data().c_str());
  return result;
  */
}

/// Return the Remus connection object this class owns.
remus::client::ServerConnection RemusBridgeConnection::connection()
{
  return this->m_conn;
}

/// Return the associated model manager's log (or a dummy log if we have no manager).
smtk::io::Logger& RemusBridgeConnection::log()
{
  static smtk::io::Logger dummy;
  return this->modelManager() ? this->modelManager()->log() : dummy;
}

/**\brief Given a Remus-style worker name (e.g., "smtk[cgm{OpenCascade}]"),
  *       find a bridge of that type.
  *
  * If none is found, a "null" shared-pointer is returned.
  */
RemusRemoteBridge::Ptr RemusBridgeConnection::findBridgeForRemusType(
  const std::string& rtype)
{
  RemusRemoteBridge::Ptr bridge;
  std::map<UUID,std::string>::const_iterator bit;
  for (bit = this->m_remoteBridgeSessionIds.begin(); bit != this->m_remoteBridgeSessionIds.end(); ++bit)
    {
    if (bit->second == rtype)
      {
      smtk::model::Bridge::Ptr bridgeBase = this->m_modelMgr->findBridgeSession(bit->first);
      bridge = smtk::dynamic_pointer_cast<RemusRemoteBridge>(bridgeBase);
      if (bridge)
        {
        return bridge;
        }
      }
    }
  return bridge;
}

/**\brief Obtain the job requirements for the named worker.
  *
  * If none exists, the method returns false and \a jreq is not set;
  * otherwise it returns true.
  */
bool RemusBridgeConnection::findRequirementsForRemusType(remus::proto::JobRequirements& jreq, const std::string& worker)
{
  std::map<std::string,std::string>::const_iterator wit =
    this->m_remoteBridgeNameToType.find(worker);
  if (wit == this->m_remoteBridgeNameToType.end())
    return false;

  remus::proto::JobRequirementsSet reqSet =
    this->m_client->retrieveRequirements(
      remus::common::MeshIOType(wit->second, "smtk::model[native]"));
  if (!reqSet.size())
    return false;
  remus::proto::JobRequirementsSet::const_iterator kit;
  for (kit = reqSet.begin(); kit != reqSet.end(); ++kit)
    {
    const remus::proto::JobRequirements& kernelInfo(*kit);
    if (kernelInfo.workerName() != worker)
      continue;

    jreq = *kit;
    return true;
    }

  return false;
}

    } // namespace remote
  } // namespace bridge
} // namespace smtk
#endif // SHIBOKEN_SKIP
