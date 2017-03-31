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
#include "smtk/bridge/remote/Session.h"
#include "smtk/bridge/remote/RemusConnection.h"

#include "smtk/io/SaveJSON.h"
#include "smtk/io/LoadJSON.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ModelEntityItem.h"

#include "smtk/model/EntityRef.h"
#include "smtk/model/RemoteOperator.h"
#include "smtk/model/SessionRegistrar.h"
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

std::map<std::string,RemusStaticSessionInfo>*
  Session::s_remotes = NULL;

Session::Session()
{
  this->initializeOperatorSystem(Session::s_operators);
}

Session::~Session()
{
}

/**\brief Initiate a new modeling session or rejoin an existing one.
  *
  * This uses the given Remus server connection (local or remote) to ask
  * it for a worker of the given type.
  */
Session::Ptr Session::setup(
  RemusConnection* remusServerConnection,
  remus::proto::JobRequirements& jreq)
{
  this->m_remusConn = remusServerConnection;
  this->m_remusWorkerReqs = jreq;
  return shared_from_this();
}

/// Return the job requirements used to identify the particular worker backing this session.
remus::proto::JobRequirements Session::remusRequirements() const
{
  return this->m_remusWorkerReqs;
}

smtk::model::SessionInfoBits Session::transcribeInternal(
  const smtk::model::EntityRef& entity, smtk::model::SessionInfoBits flags, int depth)
{
  cJSON* par;
  cJSON* req = SaveJSON::createRPCRequest("fetch-entity", par, /*id*/ "1", cJSON_Object);
  cJSON_AddItemToObject(par, "entity", cJSON_CreateString(entity.entity().toString().c_str()));
  cJSON_AddItemToObject(par, "flags", cJSON_CreateNumber(flags));
  cJSON_AddItemToObject(par, "depth", cJSON_CreateNumber(depth));

  cJSON* resp = this->m_remusConn->jsonRPCRequest(req, this->m_remusWorkerReqs);
  cJSON* err = NULL;
  cJSON* res;

  if (
    !resp ||
    (err = cJSON_GetObjectItem(resp, "error")) ||
    !(res = cJSON_GetObjectItem(resp, "result")) ||
    res->type != cJSON_True)
    {
    return smtk::model::SESSION_NOTHING;
    }

  return smtk::model::SESSION_NOTHING;
}

bool Session::ableToOperateDelegate(
  smtk::model::RemoteOperatorPtr op)
{
  cJSON* par;
  cJSON* req = SaveJSON::createRPCRequest("operator-able", par, /*id*/ "1", cJSON_Object);
  smtk::io::SaveJSON::forOperator(op->specification(), par);
  // Add the session's session ID so it can be properly instantiated on the server.
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

smtk::model::OperatorResult Session::operateDelegate(
  smtk::model::RemoteOperatorPtr op)
{
  cJSON* par;
  cJSON* req = SaveJSON::createRPCRequest("operator-apply", par, /*id*/ "1", cJSON_Object);
  smtk::io::SaveJSON::forOperator(op->specification(), par);
  // Add the session's session ID so it can be properly instantiated on the server.
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
    !smtk::io::LoadJSON::ofOperatorResult(res, result, op))
    {
    return op->createResult(smtk::model::OPERATION_FAILED);
    }
  smtk::attribute::ModelEntityItem::Ptr models = result->findModelEntity("model");
  if (models)
    {
    // Any operator that returns a special "model" item in its result
    // will have those UUIDs added to its dangling entities.
    int numModels = static_cast<int>(models->numberOfValues());
    std::cout << "Result has " << numModels << " models\n";
    for (int i = 0; i < numModels; ++i)
      {
      std::cout << "   " << models->value(i).entity().toString() << " dangling\n";
      this->declareDanglingEntity(models->value(i));
      this->transcribe(models->value(i), smtk::model::SESSION_EVERYTHING);
      }
    }

  return result;
}

void Session::cleanupSessionTypes()
{
  if (Session::s_remotes)
    {
    delete Session::s_remotes;
    Session::s_remotes = NULL;
    }
}

RemusStaticSessionInfo Session::createFunctor(
  RemusConnectionPtr remusConn,
  const remus::proto::JobRequirements& jobReq,
  const std::string& meshType)
{
  RemusStaticSessionInfo binfo(remusConn, jobReq, meshType);
  if (!Session::s_remotes)
    {
    Session::s_remotes = new std::map<std::string,RemusStaticSessionInfo>;
    atexit(Session::cleanupSessionTypes);
    }
  (*Session::s_remotes)[binfo.name()] = binfo;
  return binfo;
}

    } // namespace remote
  } // namespace bridge
} // namespace smtk

smtkImplementsModelingKernel(
  SMTKREMOTESESSION_EXPORT,
  remus_remote,
  "",
  smtk::model::SessionHasNoStaticSetup,
  smtk::bridge::remote::Session,
  false /* do not inherit local operators */
);
#endif // SHIBOKEN_SKIP
