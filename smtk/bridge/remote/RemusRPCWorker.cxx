#include "smtk/bridge/remote/RemusRPCWorker.h"
#include "smtk/bridge/remote/RemusRemoteBridge.h"

#include "smtk/model/BridgeRegistrar.h"
#include "smtk/io/ImportJSON.h"
#include "smtk/io/ExportJSON.h"
#include "smtk/model/Operator.h"

#include "remus/proto/JobContent.h"
#include "remus/proto/JobProgress.h"
#include "remus/proto/JobResult.h"
#include "remus/proto/JobStatus.h"

#include "remus/worker/Job.h"
#include "remus/worker/Worker.h"

#include "cJSON.h"

using namespace remus::proto;

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

/**\brief Evalate a JSON-RPC 2.0 request encapsulated in a Remus job.
  */
void RemusRPCWorker::processJob(
  remus::worker::Worker*& w,
  remus::worker::Job& jd,
  remus::proto::JobRequirements& r)
{
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
      std::cout << "  method \"" << methStr << "\"\n";
      bool missingIdFatal = true;
      if (reqId && reqId->type == cJSON_String)
        {
        reqIdStr = reqId->valuestring;
        missingIdFatal = false;
        }

      // I. Requests:
      //   search-bridges (available)
      //   list-bridges (instantiated)
      //   create-bridge
      //   fetch-model
      //   operator-able
      //   operator-apply

      //std::cout << "  " << methStr << "\n";
      if (methStr == "search-bridges")
        {
        smtk::model::StringList bridgeNames = this->m_modelMgr->bridgeNames();
        cJSON_AddItemToObject(result, "result",
          smtk::io::ExportJSON::createStringArray(bridgeNames));
        }
      else if (methStr == "bridge-filetypes")
        {
        cJSON* bname;
        if (
          !param ||
          !(bname = cJSON_GetObjectItem(param, "bridge-name")) ||
          bname->type != cJSON_String ||
          !bname->valuestring ||
          !bname->valuestring[0])
          {
          this->generateError(result, "Parameters not passed or bridge-name not specified.", reqIdStr);
          }
        else
          {
          RemusModelBridgeType remusType =
            RemusRemoteBridge::findAvailableType(bname->valuestring);
          if (remusType)
            {
            smtk::model::StringList bridgeFileTypes =
              this->m_modelMgr->bridgeFileTypes(remusType->bridgeName());
            cJSON_AddItemToObject(result, "result",
              smtk::io::ExportJSON::createStringArray(bridgeFileTypes));
            }
          }
        }
      else if (methStr == "create-bridge")
        {
        RemusModelBridgeType remusType;
        smtk::model::StringList bridgeNames = this->m_modelMgr->bridgeNames();
        std::set<std::string> bridgeSet(bridgeNames.begin(), bridgeNames.end());
        cJSON* bname;
        if (
          !param ||
          !(bname = cJSON_GetObjectItem(param, "bridge-name")) ||
          bname->type != cJSON_String ||
          !bname->valuestring ||
          !bname->valuestring[0] ||
          !(remusType = RemusRemoteBridge::findAvailableType(bname->valuestring)) ||
          bridgeSet.find(remusType->bridgeName()) == bridgeSet.end())
          {
          this->generateError(result,
            "Parameters not passed or bridge-name not specified/invalid.",
            reqIdStr);
          }
        else
          {
          remusType->bridgePrep();
          smtk::model::BridgeConstructor bctor =
            smtk::model::BridgeRegistrar::bridgeConstructor(remusType->bridgeName());
          if (!bctor)
            {
            this->generateError(result,
              "Unable to obtain bridge constructor", reqIdStr);
            }
          else
            {
            smtk::model::BridgePtr bridge = bctor();
            if (!bridge || bridge->sessionId().isNull())
              {
              this->generateError(result,
                "Unable to construct bridge or got NULL session ID.", reqIdStr);
              }
            else
              {
              this->m_modelMgr->registerBridgeSession(bridge);
              cJSON* sess = cJSON_CreateObject();
              smtk::io::ExportJSON::forManagerBridgeSession(
                bridge->sessionId(), sess, this->m_modelMgr);
              cJSON_AddItemToObject(result, "result", sess);
              // Now redefine our worker to be a new one whose
              // requirements include a tag for this bridge session.
              // That way it can be singled out by the client that
              // initiated the session.
              r = make_JobRequirements(
                r.meshTypes(), r.workerName(),
                r.hasRequirements() ? r.requirements() : "");
              r.tag(bridge->sessionId().toString());
              remus::worker::ServerConnection conn2 =
                remus::worker::make_ServerConnection(
                  w->connection().endpoint());
              conn2.context(w->connection().context());
              swapWorker = new remus::worker::Worker(r, conn2);
              std::cerr
                << "Redefining worker. "
                <<"Requirements now tagged with "
                << bridge->sessionId().toString() << ".\n";
              //cJSON_AddItemToObject(result, "result",
              //  cJSON_CreateString(bridge->sessionId().toString().c_str()));
              }
            }
          }
        }
      else if (methStr == "fetch-model")
        {
        cJSON* model = cJSON_CreateObject();
        // Never include bridge session list or tessellation data
        // Until someone makes us.
        smtk::io::ExportJSON::fromModel(model, this->m_modelMgr,
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
      //   delete bridge
      else if (methStr == "delete-bridge")
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
          smtk::model::BridgePtr bridge =
            this->m_modelMgr->findBridgeSession(
              smtk::common::UUID(bsess->valuestring));
          if (!bridge)
            {
            this->generateError(result, "No bridge with given session ID.", reqIdStr);
            }
          else
            {
            this->m_modelMgr->unregisterBridgeSession(bridge);
            // Remove tag from worker requirements.
            r = make_JobRequirements(
              r.meshTypes(), r.workerName(), r.hasRequirements() ? r.requirements() : "");
            swapWorker = new remus::worker::Worker(r, w->connection());
            std::cerr
              << "Redefining worker. "
              << "Requirements now untagged, removed "
              << bridge->sessionId().toString() << ".\n";
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
  //std::cout << "Response is " << response << "\n";
  w->returnMeshResults(jobResult);
  free(response);
  if (swapWorker)
    {
    //delete w;
    w = swapWorker;
    }
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
