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
using namespace smtk::io;

namespace smtk {
  namespace bridge {
    namespace remote {

std::map<std::string,RemusStaticBridgeInfo>*
  RemusRemoteBridge::s_remotes = NULL;

RemusRemoteBridge::RemusRemoteBridge()
{
  this->initializeOperatorSystem(RemusRemoteBridge::s_operators);
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
  cJSON* par;
  cJSON* req = ExportJSON::createRPCRequest("fetch-entity", par, /*id*/ "1", cJSON_Object);
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
  cJSON* par;
  cJSON* req = ExportJSON::createRPCRequest("operator-able", par, /*id*/ "1", cJSON_Object);
  smtk::io::ExportJSON::forOperator(op->specification(), par);
  // Add the bridge's session ID so it can be properly instantiated on the server.
  cJSON_AddItemToObject(par, "sessionId", cJSON_CreateString(this->sessionId().toString().c_str()));

  // Submit job to worker and wait for a response.
  // TODO: Submit job asynchronously and return immediately.
  cJSON* resp = this->m_remusConn->jsonRPCRequest(req, this->m_remusWorkerReqs);
  cJSON* err = NULL;
  cJSON* res;

  if (
    !resp ||
    (err = cJSON_GetObjectItem(resp, "error")) ||
    !(res = cJSON_GetObjectItem(resp, "result")) ||
    res->type != cJSON_True)
    {
    return false;
    }

  return true;
}

smtk::model::OperatorResult RemusRemoteBridge::operateDelegate(
  smtk::model::RemoteOperatorPtr op)
{
  cJSON* par;
  cJSON* req = ExportJSON::createRPCRequest("operator-apply", par, /*id*/ "1", cJSON_Object);
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

    } // namespace remote
  } // namespace bridge
} // namespace smtk

smtkImplementsModelingKernel(
  remus_remote,
  "",
  smtk::model::BridgeHasNoStaticSetup,
  smtk::bridge::remote::RemusRemoteBridge,
  false /* do not inherit local operators */
);
#endif // SHIBOKEN_SKIP
