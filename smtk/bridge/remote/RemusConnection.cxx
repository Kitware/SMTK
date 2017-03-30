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
#include "smtk/bridge/remote/RemusConnection.h"
#include "smtk/bridge/remote/Session.h"

#include "smtk/io/LoadJSON.h"
#include "smtk/io/SaveJSON.h"

#include "smtk/model/Operator.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/common/Environment.h"
#include "smtk/common/Paths.h"

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

namespace smtk
{
namespace bridge
{
namespace remote
{

RemusConnection::RemusConnection()
{
  Paths paths;
  StringList spaths = paths.workerSearchPaths();
  this->m_searchDirs = searchdir_t(spaths.begin(), spaths.end());
  this->m_modelMgr = smtk::model::Manager::create();
}

RemusConnection::~RemusConnection()
{
}

/**\brief Indicate location where Remus worker files may live.
  *
  * Before calling connectToServer(void), call this method
  * for each directory you would like the process-local Remus
  * server to search for worker files.
  * This has no effect once connectToServer() has been invoked.
  */
void RemusConnection::addSearchDir(const std::string& searchDir)
{
  this->m_searchDirs.insert(searchDir);
}

/**\brief Reset all of the search directories added with addSearchDir().
  *
  * If \a clearDefaultsToo is true, then the pre-existing search
  * paths provided by smtk::common::Paths::workerSearchPaths()
  * are eliminated as well.
  */
void RemusConnection::clearSearchDirs(bool clearDefaultsToo)
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
bool RemusConnection::connectToServer(const std::string& hostname, int port)
{
  // TODO: Drop any current connection and reset sessions? Copy-on-connect? ???
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
    for (searchdir_t::const_iterator it = this->m_searchDirs.begin();
         it != this->m_searchDirs.end(); ++it)
      factory->addWorkerSearchDirectory(*it);

    this->m_localServer =
      smtk::shared_ptr<remus::Server>(new remus::Server(remus::server::ServerPorts(), factory));
    this->m_localServer->startBrokering();

    // Connect to the process-local server
    const remus::server::ServerPorts& ports = this->m_localServer->serverPortInfo();

    //The server could have bound to a different port than the one requested.
    //This happens when another remus server is active, so ask the server
    //for the information on how to connect to it.
    this->m_conn = remus::client::make_ServerConnection(ports.client().endpoint());
    //since the server and client are local they can share the same context
    this->m_conn.context(ports.context());
  }
  else
  {
    this->m_conn = remus::client::ServerConnection(hostname, port);
  }

  this->m_client = smtk::shared_ptr<remus::client::Client>(new remus::client::Client(this->m_conn));
  this->m_remoteSessionNameToType.clear();
  return true;
}

/// Return the list of sessions available on the server (not the local modelManager()'s list).
std::vector<std::string> RemusConnection::sessionTypeNames()
{
  std::vector<std::string> resultVec;
  if (this->m_remoteSessionNameToType.empty())
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
          RemusStaticSessionInfo binfo =
            Session::createFunctor(shared_from_this(), kernelInfo, mit->inputType());
          // FIXME: If we implemented it, we could pass a method to
          //        accept remotely-provided pre-construction setup
          //        options to the session. But that is too fancy for now.
          smtk::model::SessionRegistrar::registerSession(binfo.name(), binfo.tags(),
            smtk::bind(&RemusStaticSessionInfo::staticSetup, binfo, _1, _2), binfo,
            SessionRegistrar::sessionOperatorConstructors(binfo.name()));
          this->m_remoteSessionNameToType[binfo.name()] = mit->inputType();
          smtkInfoMacro(log(), "Added model worker named \"" << binfo.name() << "\", type \""
                                                             << mit->inputType() << "\".");
        }
      }
    }
  }
  std::map<std::string, std::string>::const_iterator bit;
  for (bit = this->m_remoteSessionNameToType.begin(); bit != this->m_remoteSessionNameToType.end();
       ++bit)
    resultVec.push_back(bit->first);

  return resultVec;
}

/**\brief Start a worker of a particular type, or revive a particular session ID.
  *
  */
int RemusConnection::staticSetup(
  const std::string& sessionName, const std::string& optName, const smtk::model::StringList& optVal)
{
  (void)this->sessionTypeNames(); // ensure that we've fetched the session names from the server.
  if (this->m_remoteSessionNameToType.find(sessionName) == this->m_remoteSessionNameToType.end())
    return 0;

  remus::proto::JobRequirements jreq;
  if (!this->findRequirementsForRemusType(jreq, sessionName))
    return 0;

  //FIXME: Sanitize sessionName!
  cJSON* params;
  cJSON* request = SaveJSON::createRPCRequest("session-setup", params, /*id*/ "1");
  cJSON_AddItemToObject(params, "session-name", cJSON_CreateString(sessionName.c_str()));
  cJSON_AddItemToObject(params, "option-name", cJSON_CreateString(optName.c_str()));
  cJSON_AddItemToObject(params, "option-value", SaveJSON::createStringArray(optVal));
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
UUID RemusConnection::beginSession(const std::string& sessionName)
{
  (void)this->sessionTypeNames(); // ensure that we've fetched the session names from the server.
  if (this->m_remoteSessionNameToType.find(sessionName) == this->m_remoteSessionNameToType.end())
    return UUID::null();

  remus::proto::JobRequirements jreq;
  if (!this->findRequirementsForRemusType(jreq, sessionName))
    return UUID::null();

  cJSON* params;
  cJSON* req = SaveJSON::createRPCRequest("create-session", params, /*id*/ "1", cJSON_Object);
  cJSON_AddItemToObject(params, "session-name",
    cJSON_CreateString(sessionName.c_str())); //jreq.meshTypes().inputType().c_str()));

  cJSON* result = this->jsonRPCRequest(req, jreq);

  cJSON* sessionObj;
  cJSON* sessionIdObj;
  cJSON* opsObj;
  cJSON* nameObj;
  if (
    // Was JSON parsable?
    !result ||
    // Is the result an Object (as required by JSON-RPC 2.0)?
    result->type != cJSON_Object ||
    // Does the result Object have a field named "result" (req'd by JSON-RPC)?
    !(sessionObj = cJSON_GetObjectItem(result, "result")) ||
    // Is the "result" field an Object with a child that is also an object?
    sessionObj->type != cJSON_Object || !(sessionIdObj = sessionObj->child) ||
    sessionIdObj->type != cJSON_Object ||
    // Does the first child have a valid name? (This is the session ID)
    !sessionIdObj->string || !sessionIdObj->string[0] ||
    // Does the first child have fields "name" and "ops" of type String and Array?
    !(nameObj = cJSON_GetObjectItem(sessionIdObj, "name")) || nameObj->type != cJSON_String ||
    !nameObj->valuestring || !nameObj->valuestring[0] ||
    !(opsObj = cJSON_GetObjectItem(sessionIdObj, "ops")) || opsObj->type != cJSON_String)
  {
    // TODO: See if result has "error" key and report it.
    if (result)
      cJSON_Delete(result);
    return UUID::null();
  }

  // OK, construct a special "forwarding" session locally.
  Session::Ptr session = Session::create();
  session->setup(this, jreq);
  // The LoadJSON registers this session with the model manager.
  if (smtk::io::LoadJSON::ofRemoteSession(sessionIdObj, session, this->m_modelMgr))
  {
    // Failure.
  }
  //this->m_modelMgr->registerSession(session);

  cJSON_Delete(result);

  UUID sessionId = session->sessionId();
  this->m_remoteSessionRefIds[sessionId] = sessionName;
  /*
  smtkInfoMacro(log(),
    << "Updating worker \"" << jreq.workerName() << "\""
    << " tag \"" << jreq.tag() << "\".");
  jreq.tag(sessionId.toString());
  */
  session->setup(this, jreq);
  return sessionId;
}

/**\brief Shut down the worker with the given \a sessionId.
  *
  */
bool RemusConnection::endSession(const UUID& sessionId)
{
  std::map<UUID, std::string>::iterator it = this->m_remoteSessionRefIds.find(sessionId);
  if (it == this->m_remoteSessionRefIds.end())
    return false;

  // Unhook our local cmbForwardingSession representing the remote.
  smtk::model::Session::Ptr sessionBase = SessionRef(this->m_modelMgr, sessionId).session();
  Session::Ptr session = smtk::dynamic_pointer_cast<Session>(sessionBase);
  if (session)
  {
    this->m_modelMgr->unregisterSession(session);

    // Tell the server to unregister this session.
    // (Since the server's model manager should hold the only shared pointer
    // to the session, this should kill it.)
    std::string note =
      "{\"jsonrpc\":\"2.0\", \"method\":\"delete-session\", \"params\":{\"session-id\":\"" +
      sessionId.toString() + "\"}}";
    this->jsonRPCNotification(note, session->remusRequirements());
  }

  // Now remove the entry from the proxy's list of sessions.
  this->m_remoteSessionRefIds.erase(it);
  return true;
}

/**\brief Obtain a Session given its session ID.
  *
  */
Session::Ptr RemusConnection::findSession(const smtk::common::UUID& sessionId)
{
  smtk::model::Session::Ptr sess = SessionRef(this->m_modelMgr, sessionId).session();
  return smtk::dynamic_pointer_cast<Session>(sess);
}

/**\brief Return a list of file types supported by a particular session.
  */
StringData RemusConnection::supportedFileTypes(const std::string& sessionName)
{
  StringData resultMap;
  cJSON* params;
  cJSON* request =
    SaveJSON::createRPCRequest("session-filetypes", params, /*id*/ "1", cJSON_Object);
  cJSON_AddItemToObject(params, "session-name", cJSON_CreateString(sessionName.c_str()));

  // Now we need a worker to contact. If one already exists of the
  // given type, ask it. Otherwise, find a "blank" session that the
  // server advertises.
  // I. Look for an open session of the required type.
  Session::Ptr session = this->findSessionForRemusType(sessionName);
  remus::proto::JobRequirements jreq;
  if (session)
  {
    jreq = session->remusRequirements();
  }
  else if (!this->findRequirementsForRemusType(jreq, sessionName))
  {
    return resultMap;
  }
  cJSON* result = this->jsonRPCRequest(request, jreq);
  cJSON* engines;
  if (!result || result->type != cJSON_Object ||
    !(engines = cJSON_GetObjectItem(result, "result")) || engines->type != cJSON_Object)
  {
    smtkErrorMacro(this->log(), "Invalid filetype response \""
        << (result ? cJSON_Print(result) : "null") << "\"");
    // TODO: See if result has "error" key and report it.
    if (result)
      cJSON_Delete(result);
    return StringData();
  }

  smtkDebugMacro(this->log(), "Filetype response: " << cJSON_Print(engines));
  for (cJSON* engine = engines->child; (engine = engine->next);)
  {
    smtkDebugMacro(
      this->log(), "  engine: " << engine->string << " types: " << cJSON_Print(engine->child));
    if (engine->string && engine->string[0])
    {
      std::pair<std::string, StringList> keyval;
      keyval.first = engine->string;
      StringData::iterator it = resultMap.insert(keyval).first;
      smtk::io::LoadJSON::getStringArrayFromJSON(engine, it->second);
    }
  }
  cJSON_Delete(result);
  return resultMap;
}

/**\brief Read a file without requiring a pre-existing session.
  *
  * Some sessions (such as CGM) need a fileType to be specified.
  * You may leave it empty and hope the kernel will do its best
  * to infer it.
  */
smtk::model::OperatorResult RemusConnection::readFile(
  const std::string& fileName, const std::string& fileType, const std::string& sessionName)
{
  std::string actualSessionName;
  if (sessionName.empty())
  {
    (void)this->sessionTypeNames(); // ensure that we've fetched the session names from the server.
    std::map<std::string, std::string>::const_iterator bnit;
    for (bnit = this->m_remoteSessionNameToType.begin();
         bnit != this->m_remoteSessionNameToType.end() && actualSessionName.empty(); ++bnit)
    {
      StringData fileTypesForSession = this->supportedFileTypes(bnit->first);
      StringData::const_iterator dit;
      for (dit = fileTypesForSession.begin(); dit != fileTypesForSession.end(); ++dit)
      {
        StringList::const_iterator fit;
        if (dit->first != "default" && bnit->second.find(dit->first) == std::string::npos)
          continue; // skip file types for non-default engines
        // OK, either the session has only the default engine or this record matches
        // the worker's default engine:
        for (fit = dit->second.begin(); fit != dit->second.end(); ++fit)
        {
          std::string::size_type fEnd;
          std::string::size_type eEnd = fit->find(' ');
          std::string ext(*fit, 0, eEnd);
          smtkInfoMacro(log(), "Looking for \"" << ext << "\".");
          if ((fEnd = fileName.rfind(ext)) && (fileName.size() - fEnd == eEnd))
          { // matching substring is indeed at end of fileName
            actualSessionName = bnit->first;
            smtkInfoMacro(
              log(), "Found session type " << actualSessionName << " for " << fileName << ".");
            break;
          }
        }
      }
    }
  }
  else
  {
    actualSessionName = sessionName;
  }
  Session::Ptr session = this->findSessionForRemusType(actualSessionName);
  if (!session)
  { // No existing session of that type. Create a new remote session.
    UUID sessionId = this->beginSession(actualSessionName);

    session =
      smtk::dynamic_pointer_cast<Session>(SessionRef(this->m_modelMgr, sessionId).session());
  }
  if (!session)
  {
    smtkInfoMacro(
      log(), "Could not find or create session of type \"" << actualSessionName << "\".");
    return smtk::model::OperatorResult();
  }
  smtkInfoMacro(
    log(), "Found session " << session->sessionId() << " (" << actualSessionName << ").");

  smtk::model::OperatorPtr readOp = session->op("read");
  if (!readOp)
  {
    smtkInfoMacro(log(), "Could not create read operator for session"
        << " \"" << actualSessionName << "\""
        << " (" << session->sessionId() << ").");
    return smtk::model::OperatorResult();
  }

  readOp->specification()->findFile("filename")->setValue(fileName);
  smtk::attribute::StringItem::Ptr fileTypeItem = readOp->specification()->findString("filetype");
  if (fileTypeItem)
  {
    fileTypeItem->setValue(fileType);
  }

  smtk::model::OperatorResult result = readOp->operate();

  // Fetch affected models
  smtk::attribute::ModelEntityItem::Ptr models = result->findModelEntity("model");
  if (models)
  {
    int numModels = static_cast<int>(models->numberOfValues());
    for (int i = 0; i < numModels; ++i)
      this->fetchWholeModel(models->value(i).entity());
  }
  this->m_modelMgr->assignDefaultNames();
  return result;
}

/**\brief
  *
  */
std::vector<std::string> RemusConnection::operatorNames(const std::string& sessionName)
{
  (void)sessionName;
  std::vector<std::string> result;
  return result;
}

/**\brief
  *
  */
std::vector<std::string> RemusConnection::operatorNames(const UUID& sessionId)
{
  (void)sessionId;
  std::vector<std::string> result;
  return result;
}

/**\brief
  *
  */
smtk::model::OperatorPtr RemusConnection::createOperator(
  const UUID& sessionOrModel, const std::string& opName)
{
  (void)opName;
  (void)sessionOrModel;
  smtk::model::OperatorPtr empty;
  return empty;
}

/**\brief
  *
  */
smtk::model::OperatorPtr RemusConnection::createOperator(
  const std::string& sessionName, const std::string& opName)
{
  (void)opName;
  (void)sessionName;
  smtk::model::OperatorPtr empty;
  return empty;
}

/**\brief Ask the current worker to return all of the entities at the remote end.
  *
  * Warning: This will overwrite this->m_modelMgr.
  */
void RemusConnection::fetchWholeModel(const UUID& modelId)
{
  // Find session from modelId, then job requirements from session.
  smtk::model::SessionRef sref = smtk::model::Model(this->m_modelMgr, modelId).session();
  smtk::model::Session::Ptr sessionBase = sref.session();
  Session::Ptr session = smtk::dynamic_pointer_cast<Session>(sessionBase);
  if (!session)
    return;

  cJSON* params;
  cJSON* request = SaveJSON::createRPCRequest("fetch-model", params, /*id*/ "1", cJSON_Array);
  cJSON* response = this->jsonRPCRequest(request, session->remusRequirements());
  cJSON* model;
  cJSON* topo;
  //smtkInfoMacro(log(), " ----- \n\n\n" << cJSON_Print(response) << "\n ----- \n\n.");
  if (response && (model = cJSON_GetObjectItem(response, "result")) &&
    model->type == cJSON_Object && (topo = cJSON_GetObjectItem(model, "topo")))
    smtk::io::LoadJSON::ofManager(topo, this->m_modelMgr);
  cJSON_Delete(response);
}

smtk::model::ManagerPtr RemusConnection::modelManager()
{
  return this->m_modelMgr;
}

void RemusConnection::setModelManager(smtk::model::ManagerPtr mgr)
{
  this->m_modelMgr = mgr;
}

cJSON* RemusConnection::jsonRPCRequest(cJSON* req, const remus::proto::JobRequirements& jreq)
{
  char* reqStr = cJSON_Print(req);
  cJSON_Delete(req);
  cJSON* response = this->jsonRPCRequest(reqStr, jreq);
  free(reqStr);
  return response;
}

/// Perform a synchronous JSON-RPC request.
cJSON* RemusConnection::jsonRPCRequest(
  const std::string& request, const remus::proto::JobRequirements& jreq)
{
  remus::proto::JobContent jcnt(remus::common::ContentFormat::JSON, request);
  remus::proto::JobSubmission jsub(jreq);
  jsub.insert(remus::proto::JobSubmission::value_type(
    "request", remus::proto::JobContent(remus::common::ContentFormat::JSON, request)));
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

void RemusConnection::jsonRPCNotification(cJSON* note, const remus::proto::JobRequirements& jreq)
{
  char* noteStr = cJSON_Print(note);
  cJSON_Delete(note);
  this->jsonRPCNotification(noteStr, jreq);
  free(noteStr);
}

void RemusConnection::jsonRPCNotification(
  const std::string& note, const remus::proto::JobRequirements& jreq)
{
  remus::proto::JobContent jcnt(remus::common::ContentFormat::JSON, note);
  remus::proto::JobSubmission jsub(jreq);
  jsub.insert(remus::proto::JobSubmission::value_type(
    "request", remus::proto::JobContent(remus::common::ContentFormat::JSON, note)));
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
remus::client::ServerConnection RemusConnection::connection()
{
  return this->m_conn;
}

/// Return the associated model manager's log (or a dummy log if we have no manager).
smtk::io::Logger& RemusConnection::log()
{
  static smtk::io::Logger dummy;
  return this->modelManager() ? this->modelManager()->log() : dummy;
}

/**\brief Given a Remus-style worker name (e.g., "smtk[cgm{OpenCascade}]"),
  *       find a session of that type.
  *
  * If none is found, a "null" shared-pointer is returned.
  */
Session::Ptr RemusConnection::findSessionForRemusType(const std::string& rtype)
{
  Session::Ptr session;
  std::map<UUID, std::string>::const_iterator bit;
  for (bit = this->m_remoteSessionRefIds.begin(); bit != this->m_remoteSessionRefIds.end(); ++bit)
  {
    if (bit->second == rtype)
    {
      smtk::model::Session::Ptr sessionBase = SessionRef(this->m_modelMgr, bit->first).session();
      session = smtk::dynamic_pointer_cast<Session>(sessionBase);
      if (session)
      {
        return session;
      }
    }
  }
  return session;
}

/**\brief Obtain the job requirements for the named worker.
  *
  * If none exists, the method returns false and \a jreq is not set;
  * otherwise it returns true.
  */
bool RemusConnection::findRequirementsForRemusType(
  remus::proto::JobRequirements& jreq, const std::string& worker)
{
  std::map<std::string, std::string>::const_iterator wit =
    this->m_remoteSessionNameToType.find(worker);
  if (wit == this->m_remoteSessionNameToType.end())
    return false;

  remus::proto::JobRequirementsSet reqSet = this->m_client->retrieveRequirements(
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
