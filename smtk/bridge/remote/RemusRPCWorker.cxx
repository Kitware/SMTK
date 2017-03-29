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
#include "smtk/bridge/remote/RemusRPCWorker.h"
#include "smtk/bridge/remote/Session.h"

#include "smtk/io/ExportJSON.h"
#include "smtk/io/ImportJSON.h"

#include "smtk/model/Operator.h"
#include "smtk/model/SessionRegistrar.h"

#include "smtk/common/StringUtil.h"

#include "remus/proto/JobContent.h"
#include "remus/proto/JobProgress.h"
#include "remus/proto/JobResult.h"
#include "remus/proto/JobStatus.h"

#include "remus/worker/Job.h"
#include "remus/worker/Worker.h"

#include "cJSON.h"

#include <algorithm>
#include <cctype>
#include <functional>
#include <locale>
#include <sstream>

using namespace remus::proto;
using namespace smtk::model;
using namespace smtk::common;

namespace smtk {
  namespace bridge {
    namespace remote {

RemusRPCWorker::RemusRPCWorker()
{
  this->m_modelMgr = smtk::model::Manager::create();
}

RemusRPCWorker::~RemusRPCWorker()
{
}

/**\brief Set an option to be used by the worker as it processes jobs.
  *
  * Options currently recognized include "default_kernel", "default_engine",
  * "exclude_kernels", and "exclude_engines".
  * These are used to constrain the worker to a specific modeler.
  *
  * The first two options are used to solve dilemmas where a file
  * to be read or other operation to be performed might feasibly be
  * executed using different kernels or engines.
  * When a tie occurs, the defaults are used.
  *
  * The latter two options are used to prevent the application from
  * accessing functionality built into SMTK but administratively
  * prohibited (for example, due to stability problems or licensing
  * issues).
  * The exclusion rules are not applied to values in the default
  * kernel and engine, so specifying the wildcard "*" for both
  * the kernels and engines will prohibit any but the default
  * from being used.
  * Otherwise the "exclude_*" options should be comma-separated lists.
  */
void RemusRPCWorker::setOption(
  const std::string& optName,
  const std::string& optVal)
{
  smtkDebugMacro(this->manager()->log(),
    "set option \"" << optName << "\" to \"" << optVal << "\"");

  StringList vals;
  if (
    optName.find("exclude_") == 0 && (
      optName == "exclude_kernels" ||
      optName == "exclude_engines"))
    {
    std::stringstream stream(optVal);
    while (stream.good())
      {
      std::string token;
      std::getline(stream, token, ',');
      vals.push_back(StringUtil::trim(token));
      }
    }
  else
    {
    vals.push_back(optVal);
    }
  this->m_options[optName] = vals;
}

/// Remove all options recorded by setOption.
void RemusRPCWorker::clearOptions()
{
  this->m_options.clear();
}

/**\brief Evalate a JSON-RPC 2.0 request encapsulated in a Remus job.
  */
void RemusRPCWorker::processJob(
  remus::worker::Worker*& w,
  remus::worker::Job& jd,
  remus::proto::JobRequirements& r)
{
  (void)r;
  JobProgress progress;
  JobStatus status(jd.id(),remus::IN_PROGRESS);

  // Mark start of job
  progress.setValue(1);
  status.updateProgress(progress);
  w->updateStatus(status);

  remus::worker::Worker* swapWorker = NULL;
  cJSON* result = cJSON_CreateObject();
  std::string content = jd.details("request");
  if (content.empty())
    {
    this->generateError(result, "No request", "");
    }
  else
    {
    cJSON* req = cJSON_Parse(content.c_str());
    smtkDebugMacro(this->manager()->log(),
      "job \"" << content << "\"");
    cJSON* spec;
    cJSON* meth;
    if (
      !req ||
      req->type != cJSON_Object ||
      !(meth = cJSON_GetObjectItem(req, "method")) ||
      meth->type != cJSON_String ||
      !meth->valuestring ||
      !(spec = cJSON_GetObjectItem(req, "jsonrpc")) ||
      spec->type != cJSON_String ||
      !spec->valuestring)
      {
      this->generateError(
        result, "Malformed request; not an object or missing jsonrpc or method members.", "");
      }
    else
      {
      std::string methStr(meth->valuestring);
      std::string specStr(spec->valuestring);
      std::string reqIdStr;
      cJSON* reqId = cJSON_GetObjectItem(req, "id");
      cJSON* param = cJSON_GetObjectItem(req, "params");
      smtkDebugMacro(this->manager()->log(),
        "method \"" << methStr << "\"");
      bool missingIdFatal = true;
      if (reqId && reqId->type == cJSON_String)
        {
        reqIdStr = reqId->valuestring;
        missingIdFatal = false;
        }

      // I. Requests:
      //   search-sessions (available)
      //   list-sessions (instantiated)
      //   create-session
      //   fetch-model
      //   operator-able
      //   operator-apply

      //smtkDebugMacro(this->manager()->log(), "  " << methStr);
      if (methStr == "search-sessions")
        {
        smtk::model::StringList sessionTypeNames = this->m_modelMgr->sessionTypeNames();
        cJSON_AddItemToObject(result, "result",
          smtk::io::ExportJSON::createStringArray(sessionTypeNames));
        }
      else if (methStr == "session-filetypes")
        {
        cJSON* bname;
        if (
          !param ||
          !(bname = cJSON_GetObjectItem(param, "session-name")) ||
          bname->type != cJSON_String ||
          !bname->valuestring ||
          !bname->valuestring[0])
          {
          this->generateError(result, "Parameters not passed or session-name not specified.", reqIdStr);
          }
        else
          {
          cJSON* typeObj = cJSON_CreateObject();
          smtk::model::StringData sessionFileTypes =
            SessionRegistrar::sessionFileTypes(bname->valuestring);
          for(PropertyNameWithStrings it = sessionFileTypes.begin();
              it != sessionFileTypes.end(); ++it)
            {
            if(it->second.size())
              cJSON_AddItemToObject(typeObj, it->first.c_str(),
                smtk::io::ExportJSON::createStringArray(it->second));
            }
          cJSON_AddItemToObject(result, "result", typeObj);
          }
        }
      else if (methStr == "create-session")
        {
        smtk::model::StringList sessionTypeNames = this->m_modelMgr->sessionTypeNames();
        std::set<std::string> sessionSet(sessionTypeNames.begin(), sessionTypeNames.end());
        cJSON* bname;
        if (
          !param ||
          !(bname = cJSON_GetObjectItem(param, "session-name")) ||
          bname->type != cJSON_String ||
          !bname->valuestring ||
          !bname->valuestring[0] ||
          sessionSet.find(bname->valuestring) == sessionSet.end())
          {
          this->generateError(result,
            "Parameters not passed or session-name not specified/invalid.",
            reqIdStr);
          }
        else
          {
          // Pass options such as engine name (if any) to static setup
          smtk::model::SessionStaticSetup bsetup =
            smtk::model::SessionRegistrar::sessionStaticSetup(bname->valuestring);
          cJSON* ename;
          if (
            bsetup &&
            (ename = cJSON_GetObjectItem(param, "engine-name")) &&
            ename->type == cJSON_String &&
            !ename->valuestring && !ename->valuestring[0])
            {
            std::string defEngine = ename->valuestring;
            if (!defEngine.empty())
              {
              StringList elist;
              elist.push_back(ename->valuestring);
              bsetup("engine", elist);
              }
            }

          smtk::model::SessionConstructor bctor =
            smtk::model::SessionRegistrar::sessionConstructor(bname->valuestring);
          if (!bctor)
            {
            this->generateError(result,
              "Unable to obtain session constructor", reqIdStr);
            }
          else
            {
            smtk::model::SessionPtr session = bctor();
            if (!session || session->sessionId().isNull())
              {
              this->generateError(result,
                "Unable to construct session or got NULL session ID.", reqIdStr);
              }
            else
              {
              this->m_modelMgr->registerSession(session);
              cJSON* sess = cJSON_CreateObject();
              smtk::io::ExportJSON::forManagerSession(
                session->sessionId(), sess, this->m_modelMgr);
              cJSON_AddItemToObject(result, "result", sess);
              }
            }
          }
        }
      else if (methStr == "fetch-model")
        {
        cJSON* model = cJSON_CreateObject();
        // Never include session list or tessellation data
        // Until someone makes us.
        smtk::io::ExportJSON::fromModelManager(model, this->m_modelMgr,
          static_cast<smtk::io::JSONFlags>(
            smtk::io::JSON_ENTITIES | smtk::io::JSON_PROPERTIES));
        cJSON_AddItemToObject(result, "result", model);
        }
      else if (methStr == "operator-able")
        {
        smtk::model::OperatorPtr localOp;
        if (
          !param ||
          !smtk::io::ImportJSON::ofOperator(param, localOp, this->m_modelMgr) ||
          !localOp)
          {
          this->generateError(result,
            "Parameters not passed or invalid operator specified.",
            reqIdStr);
          }
        else
          {
          bool able = localOp->ableToOperate();
          cJSON_AddItemToObject(result, "result", cJSON_CreateBool(able ? 1 : 0));
          }
        }
      else if (methStr == "operator-apply")
        {
        smtk::model::OperatorPtr localOp;
        if (
          !param ||
          !smtk::io::ImportJSON::ofOperator(param, localOp, this->m_modelMgr) ||
          !localOp)
          {
          this->generateError(result,
            "Parameters not passed or invalid operator specified.",
            reqIdStr);
          }
        else
          {
          smtk::model::OperatorResult ores = localOp->operate();
          cJSON* oresult = cJSON_CreateObject();
          smtk::io::ExportJSON::forOperatorResult(ores, oresult);
          cJSON_AddItemToObject(result, "result", oresult);
          }
        }
      // II. Notifications:
      //   delete session
      else if (methStr == "delete-session")
        {
        missingIdFatal &= false; // Notifications do not require an "id" member in the request.

        cJSON* bsess;
        if (
          !param ||
          !(bsess = cJSON_GetObjectItem(param, "session-id")) ||
          bsess->type != cJSON_String ||
          !bsess->valuestring ||
          !bsess->valuestring[0])
          {
          this->generateError(result, "Parameters not passed or session-id not specified/invalid.", reqIdStr);
          }
        else
          {
          smtk::model::SessionPtr session =
            SessionRef(
              this->m_modelMgr,
              smtk::common::UUID(bsess->valuestring)
            ).session();
          if (!session)
            {
            this->generateError(result, "No session with given session ID.", reqIdStr);
            }
          else
            {
            this->m_modelMgr->unregisterSession(session);
            }
          }
        }
      if (missingIdFatal)
        {
        this->generateError(result, "Method was a request but is missing \"id\".", reqIdStr);
        }
      }
    }

  progress.setValue(100);
  //progress.setMessage("Mmph. Yeah!");
  status.updateProgress(progress);
  w->updateStatus(status);

  char* response = cJSON_Print(result);
  cJSON_Delete(result);
  remus::proto::JobResult jobResult =
    remus::proto::make_JobResult(
      jd.id(), response, remus::common::ContentFormat::JSON);
  smtkDebugMacro(this->manager()->log(), "Response is \"" << response << "\"");
  w->returnResult(jobResult);
  free(response);
  if (swapWorker)
    {
    //delete w;
    w = swapWorker;
    }
}

/// Return the model manager used by the worker. This should never be NULL.
smtk::model::ManagerPtr RemusRPCWorker::manager()
{
  return this->m_modelMgr;
}

/**\brief Set the model manager used by the worker. This is ignored if NULL.
  *
  * \warning
  * This should only be called immediately after creating the worker,
  * before any operations have been run.
  */
void RemusRPCWorker::setManager(smtk::model::ManagerPtr mgr)
{
  if (mgr)
    this->m_modelMgr = mgr;
}

void RemusRPCWorker::generateError(cJSON* err, const std::string& errMsg, const std::string& reqId)
{
  cJSON* result = cJSON_CreateObject();
  cJSON_AddItemToObject(err, "result", result);
  cJSON_AddItemToObject(result, "error", cJSON_CreateString(errMsg.c_str()));
  cJSON_AddItemToObject(result, "id", cJSON_CreateString(reqId.c_str()));
}

    } // namespace remote
  } // namespace bridge
} // namespace smtk
#endif // SHIBOKEN_SKIP
