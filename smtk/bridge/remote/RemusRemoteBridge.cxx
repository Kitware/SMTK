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
#include "smtk/bridge/remote/RemusRemoteBridge.h"
#include "smtk/bridge/remote/RemusBridgeConnection.h"

#include "smtk/io/ImportJSON.h"
#include "smtk/io/ExportJSON.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ModelEntityItem.h"

#include "smtk/model/BridgeRegistrar.h"
#include "smtk/model/Cursor.h"
#include "smtk/model/RemoteOperator.h"
#include "smtk/model/StringData.h"

#include "smtk/Options.h"

#include "remus/common/MeshIOType.h"
#include "remus/proto/JobSubmission.h"

#include "cJSON.h"

using namespace smtk::common;
using namespace smtk::model;

namespace smtk {
  namespace bridge {
    namespace remote {

std::map<std::string,RemusStaticBridgeInfo>*
  RemusRemoteBridge::s_remotes = NULL;

RemusRemoteBridge::RemusRemoteBridge()
{
}

RemusRemoteBridge::~RemusRemoteBridge()
{
}

/**\brief Initiate a new modeling session or rejoin an existing one.
  *
  * This uses the given Remus server connection (local or remote) to ask
  * it for a worker of the given type.
  */
RemusRemoteBridge::Ptr RemusRemoteBridge::setup(
  RemusBridgeConnection* remusServerConnection,
  remus::proto::JobRequirements& jreq)
{
  this->m_remusConn = remusServerConnection;
  this->m_remusWorkerReqs = jreq;
  return shared_from_this();
}

/// Return the job requirements used to identify the particular worker backing this bridge.
remus::proto::JobRequirements RemusRemoteBridge::remusRequirements() const
{
  return this->m_remusWorkerReqs;
}

smtk::model::BridgedInfoBits RemusRemoteBridge::transcribeInternal(
  const smtk::model::Cursor& entity, smtk::model::BridgedInfoBits flags)
{
  cJSON* req = cJSON_CreateObject();
  cJSON* par = cJSON_CreateObject();
  cJSON_AddItemToObject(req, "jsonrpc", cJSON_CreateString("2.0"));
  cJSON_AddItemToObject(req, "method", cJSON_CreateString("fetch-entity"));
  cJSON_AddItemToObject(req, "id", cJSON_CreateString("1")); // TODO
  cJSON_AddItemToObject(req, "params", par);
  cJSON_AddItemToObject(par, "entity", cJSON_CreateString(entity.entity().toString().c_str()));
  cJSON_AddItemToObject(par, "flags", cJSON_CreateNumber(flags));

  cJSON* resp = this->m_remusConn->jsonRPCRequest(req, this->m_remusWorkerReqs);
  cJSON* err = NULL;
  cJSON* res;

  if (
    !resp ||
    (err = cJSON_GetObjectItem(resp, "error")) ||
    !(res = cJSON_GetObjectItem(resp, "result")) ||
    res->type != cJSON_True)
    {
    return smtk::model::BRIDGE_NOTHING;
    }

  return smtk::model::BRIDGE_NOTHING;
}

bool RemusRemoteBridge::ableToOperateDelegate(
  smtk::model::RemoteOperatorPtr op)
{
  cJSON* req = cJSON_CreateObject();
  cJSON* par = cJSON_CreateObject();
  cJSON_AddItemToObject(req, "jsonrpc", cJSON_CreateString("2.0"));
  cJSON_AddItemToObject(req, "method", cJSON_CreateString("operator-able"));
  cJSON_AddItemToObject(req, "id", cJSON_CreateString("1")); // TODO
  cJSON_AddItemToObject(req, "params", par);
  smtk::io::ExportJSON::forOperator(op->specification(), par);
  // Add the bridge's session ID so it can be properly instantiated on the server.
  cJSON_AddItemToObject(par, "sessionId", cJSON_CreateString(this->sessionId().toString().c_str()));

  // Submit job to worker and wait for a response.
  // TODO: Submit job asynchronously and return immediately.
  cJSON* resp = this->m_remusConn->jsonRPCRequest(req, this->m_remusWorkerReqs);
  cJSON* err = NULL;
  cJSON* res;
  cJSON* data;
  cJSON* flagsOut;

  if (
    !resp ||
    (err = cJSON_GetObjectItem(resp, "error")) ||
    !(res = cJSON_GetObjectItem(resp, "result")) ||
    res->type != cJSON_Object ||
    !(data = cJSON_GetObjectItem(res, "data")) ||
    data->type != cJSON_Object ||
    !(flagsOut = cJSON_GetObjectItem(res, "flags")) ||
    flagsOut->type != cJSON_Number)
    {
    return false;
    }

  return true;
}

smtk::model::OperatorResult RemusRemoteBridge::operateDelegate(
  smtk::model::RemoteOperatorPtr op)
{
  cJSON* req = cJSON_CreateObject();
  cJSON* par = cJSON_CreateObject();
  cJSON_AddItemToObject(req, "jsonrpc", cJSON_CreateString("2.0"));
  cJSON_AddItemToObject(req, "method", cJSON_CreateString("operator-apply"));
  cJSON_AddItemToObject(req, "id", cJSON_CreateString("1")); // TODO
  cJSON_AddItemToObject(req, "params", par);
  smtk::io::ExportJSON::forOperator(op->specification(), par);
  // Add the bridge's session ID so it can be properly instantiated on the server.
  cJSON_AddItemToObject(par, "sessionId", cJSON_CreateString(this->sessionId().toString().c_str()));

  cJSON* resp = this->m_remusConn->jsonRPCRequest(req, this->m_remusWorkerReqs);
  //cJSON* resp = NULL; // this->m_proxy->jsonRPCRequest(req, this->m_remusWorkerReqs); // This deletes req and par.
  cJSON* err = NULL;
  cJSON* res;
  smtk::model::OperatorResult result;

  if (
    !resp ||
    (err = cJSON_GetObjectItem(resp, "error")) ||
    !(res = cJSON_GetObjectItem(resp, "result")) ||
    !smtk::io::ImportJSON::ofOperatorResult(res, result, op->bridge()->operatorSystem()))
    {
    return op->createResult(smtk::model::OPERATION_FAILED);
    }
  smtk::attribute::ModelEntityItem::Ptr models = result->findModelEntity("model");
  if (models)
    {
    // Any operator that returns a special "model" item in its result
    // will have those UUIDs added to its dangling entities.
    int numModels = models->numberOfValues();
    std::cout << "Result has " << numModels << " models\n";
    for (int i = 0; i < numModels; ++i)
      {
      std::cout << "   " << models->value(i).entity().toString() << " dangling\n";
      this->declareDanglingEntity(models->value(i));
      this->transcribe(models->value(i), smtk::model::BRIDGE_EVERYTHING);
      }
    }

  return result;
}

void RemusRemoteBridge::cleanupBridgeTypes()
{
  if (RemusRemoteBridge::s_remotes)
    {
    delete RemusRemoteBridge::s_remotes;
    RemusRemoteBridge::s_remotes = NULL;
    }
}

RemusStaticBridgeInfo RemusRemoteBridge::createFunctor(
  RemusBridgeConnectionPtr remusConn,
  const remus::proto::JobRequirements& jobReq,
  const std::string& meshType)
{
  RemusStaticBridgeInfo binfo(remusConn, jobReq, meshType);
  if (!RemusRemoteBridge::s_remotes)
    {
    RemusRemoteBridge::s_remotes = new std::map<std::string,RemusStaticBridgeInfo>;
    atexit(RemusRemoteBridge::cleanupBridgeTypes);
    }
  (*RemusRemoteBridge::s_remotes)[binfo.name()] = binfo;
  return binfo;
}

// NB: We do not invoke
// smtkImplementsModelingKernel(remus_remote,remusRemoteNoFileTypes,smtk::bridge::remote::RemusRemoteBridge);
// because each instance of the bridge may advertise different capabilities.
// Instead, when setup() is called with a pointer to a remus server, we query it to discover
// different bridges we can back and register each of them with a combined bridge name.
// #define smtkImplementsModelingKernel(Comp, FileTypes, Cls)

#if 0
/* Adapt create() to return a base-class pointer */
static smtk::model::BridgePtr baseCreate() {
  return RemusRemoteBridge::create();
}

/* Implement autoinit methods */
void smtk_remus_remote_bridge_AutoInit_Construct()
{
  StringList tags; // empty
  StringList fileTypes; // empty
  smtk::model::BridgeRegistrar::registerBridge(
    "remus_remote", /* Can't rely on bridgeName to be initialized yet */
    tags,
    fileTypes,
    baseCreate);
}

void smtk_remus_remote_bridge_AutoInit_Destruct()
{
  smtk::model::BridgeRegistrar::registerBridge(
    RemusRemoteBridge::bridgeName,
    std::vector<std::string>(),
    std::vector<std::string>(),
    NULL);
}

/**\brief Declare the component name */
std::string RemusRemoteBridge::bridgeName("remus_remote");

/**\brief Declare the class name */
std::string RemusRemoteBridge::className() const { return "smtk::bridge::remote::RemusRemoteBridge"; };

/**\brief Declare the map of operator constructors */
smtk::model::OperatorConstructors* RemusRemoteBridge::s_operators = NULL;

/**\brief Virtual method to allow operators to register themselves with us */
bool RemusRemoteBridge::registerOperator(
  const std::string& opName, const char* opDescrXML,
  smtk::model::OperatorConstructor opCtor)
{
  return RemusRemoteBridge::registerStaticOperator(opName, opDescrXML, opCtor);
}

/**\brief Allow operators to register themselves with us
  *
  * Normally this method is implemented by smtkImplementsModelingKernel()
  * but the RemusRemoteBridge class provides a different implementation
  * so that operators from different bridges are kept separate.
  */
bool RemusRemoteBridge::registerStaticOperator(
  const std::string& opName, const char* opDescrXML,
  smtk::model::OperatorConstructor opCtor)
{
  return RemusRemoteBridge::registerBridgedOperator(
    "remus_remote", opName, opDescrXML, opCtor);
}

bool RemusRemoteBridge::registerBridgedOperator(
  const std::string& bridgeName, const std::string& opName,
  const char* opDescrXML, smtk::model::OperatorConstructor opCtor)
{
  if (!RemusRemoteBridge::s_operators)
    {
    RemusRemoteBridge::s_operators = new smtk::model::OperatorConstructors;
    atexit(RemusRemoteBridge::cleanupOperators);
    }
  if (!opName.empty() && opCtor)
    {
    smtk::model::StaticOperatorInfo entry(opDescrXML ? opDescrXML : "",opCtor);
    (* RemusRemoteBridge::s_operators)[opName] = entry;
    return true;
    }
  else if (!opName.empty())
    { /* unregister the operator of the given name. */
    RemusRemoteBridge::s_operators->erase(opName);
    /* FIXME: We should ensure that no operator instances of this type are in */
    /*        existence before allowing "unregistration" to proceed. */
    }
  return false;
}

/**\brief Find an operator constructor in this subclass' static list. */
smtk::model::OperatorConstructor RemusRemoteBridge::findOperatorConstructor(
  const std::string& opName) const
{
  return this->findOperatorConstructorInternal(opName, RemusRemoteBridge::s_operators);
}

/**\brief Find an XML description of an operator in this subclass' static list. */
std::string RemusRemoteBridge::findOperatorXML(const std::string& opName) const
{
  return this->findOperatorXMLInternal(opName, RemusRemoteBridge::s_operators);
}

/**\brief Called to delete registered operator map at exit. */
void RemusRemoteBridge::cleanupOperators()
{
  delete RemusRemoteBridge::s_operators;
  RemusRemoteBridge::s_operators = NULL;
}
#endif // 0

    } // namespace remote
  } // namespace bridge
} // namespace smtk

smtkImplementsModelingKernel(
  remus_remote,
  "",
  smtk::model::BridgeHasNoStaticSetup,
  smtk::bridge::remote::RemusRemoteBridge);
#endif // SHIBOKEN_SKIP
