#include "vtkModelManagerWrapper.h"

#include "smtk/model/ImportJSON.h"
#include "smtk/model/ExportJSON.h"
#include "smtk/model/Operator.h"

#include "vtkObjectFactory.h"

#include "cJSON.h"

vtkStandardNewMacro(vtkModelManagerWrapper);

vtkModelManagerWrapper::vtkModelManagerWrapper()
{
  this->ModelMgr = smtk::model::Manager::create();
  this->JSONRequest = NULL;
  this->JSONResponse = NULL;
}

vtkModelManagerWrapper::~vtkModelManagerWrapper()
{
  this->SetJSONRequest(NULL);
  this->SetJSONResponse(NULL);
}

void vtkModelManagerWrapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "JSONRequest:" << this->JSONRequest << "\n";
  os << indent << "JSONResponse:" << this->JSONResponse << "\n";
  os << indent << "ModelMgr:" << this->ModelMgr.get() << "\n";
}

/**\brief Evalate a JSON-RPC 2.0 request.
  *
  * This evaluates the request present in the JSONRequest member
  * and, if the request contains an "id" field, stores a response
  * in the JSONResponse member.
  */
void vtkModelManagerWrapper::ProcessJSONRequest()
{
  cJSON* result = cJSON_CreateObject();
  if (!this->JSONRequest || !this->JSONRequest[0])
    {
    this->GenerateError(result, "No request", "");
    }
  else
    {
    cJSON* req = cJSON_Parse(this->JSONRequest);
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
      this->GenerateError(
        result, "Malformed request; not an object or missing jsonrpc or method members.", "");
      }
    else
      {
      std::string methStr(meth->valuestring);
      std::string specStr(spec->valuestring);
      std::string reqIdStr;
      cJSON* reqId = cJSON_GetObjectItem(req, "id");
      cJSON* param = cJSON_GetObjectItem(req, "params");
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
      if (methStr == "search-bridges")
        {
        smtk::model::StringList bridgeNames = this->ModelMgr->bridgeNames();
        cJSON_AddItemToObject(result, "result",
          smtk::model::ExportJSON::createStringArray(bridgeNames));
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
          this->GenerateError(result, "Parameters not passed or bridge-name not specified.", reqIdStr);
          }
        else
          {
          smtk::model::StringList bridgeFileTypes =
            this->ModelMgr->bridgeFileTypes(bname->valuestring);
          cJSON_AddItemToObject(result, "result",
            smtk::model::ExportJSON::createStringArray(bridgeFileTypes));
          }
        }
      else if (methStr == "create-bridge")
        {
        smtk::model::StringList bridgeNames = this->ModelMgr->bridgeNames();
        std::set<std::string> bridgeSet(bridgeNames.begin(), bridgeNames.end());
        cJSON* bname;
        if (
          !param ||
          !(bname = cJSON_GetObjectItem(param, "bridge-name")) ||
          bname->type != cJSON_String ||
          !bname->valuestring ||
          !bname->valuestring[0] ||
          bridgeSet.find(bname->valuestring) == bridgeSet.end())
          {
          this->GenerateError(result,
            "Parameters not passed or bridge-name not specified/invalid.",
            reqIdStr);
          }
        else
          {
          smtk::model::BridgeConstructor bctor =
            this->ModelMgr->bridgeConstructor(bname->valuestring);
          if (!bctor)
            {
            this->GenerateError(result,
              "Unable to obtain bridge constructor", reqIdStr);
            }
          else
            {
            smtk::model::BridgePtr bridge = bctor();
            if (!bridge || bridge->sessionId().isNull())
              {
              this->GenerateError(result,
                "Unable to construct bridge or got NULL session ID.", reqIdStr);
              }
            else
              {
              this->ModelMgr->registerBridgeSession(bridge);
              cJSON* sess = cJSON_CreateObject();
              smtk::model::ExportJSON::forManagerBridgeSession(
                bridge->sessionId(), sess, this->ModelMgr);
              cJSON_AddItemToObject(result, "result", sess);
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
        smtk::model::ExportJSON::fromModel(model, this->ModelMgr,
          static_cast<smtk::model::JSONFlags>(
            smtk::model::JSON_ENTITIES | smtk::model::JSON_PROPERTIES));
        cJSON_AddItemToObject(result, "result", model);
        }
      else if (methStr == "operator-able")
        {
        smtk::model::OperatorPtr localOp;
        if (
          !param ||
          !smtk::model::ImportJSON::ofOperator(param, localOp, this->ModelMgr) ||
          !localOp)
          {
          this->GenerateError(result,
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
          !smtk::model::ImportJSON::ofOperator(param, localOp, this->ModelMgr) ||
          !localOp)
          {
          this->GenerateError(result,
            "Parameters not passed or invalid operator specified.",
            reqIdStr);
          }
        else
          {
          smtk::model::OperatorResult ores = localOp->operate();
          cJSON* oresult = cJSON_CreateObject();
          smtk::model::ExportJSON::forOperatorResult(ores, oresult);
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
          this->GenerateError(result, "Parameters not passed or session-id not specified/invalid.", reqIdStr);
          }
        else
          {
          smtk::model::BridgePtr bridge =
            this->ModelMgr->findBridgeSession(
              smtk::util::UUID(bsess->valuestring));
          if (!bridge)
            {
            this->GenerateError(result, "No bridge with given session ID.", reqIdStr);
            }
          else
            {
            this->ModelMgr->unregisterBridgeSession(bridge);
            }
          }
        }
      if (missingIdFatal)
        {
        this->GenerateError(result, "Method was a request but is missing \"id\".", reqIdStr);
        }
      }
    }
  char* response = cJSON_Print(result);
  cJSON_Delete(result);
  this->SetJSONResponse(response);
  free(response);
}


/**\brief Deserializes a JSON operator description and executes ableToOperate.
  *
  * The return value is a JSON serialization of the OperatorOutcome.
  */
std::string vtkModelManagerWrapper::CanOperatorExecute(const std::string& jsonOperator)
{
  return "{\"outcome\": \"0\"}";
}

/**\brief Deserializes a JSON operator description and executes operate().
  *
  * The return value is a JSON serialization of the OperatorOutcome.
  */
std::string vtkModelManagerWrapper::ApplyOperator(const std::string& jsonOperator)
{
  return "{\"outcome\": \"0\"}";
}

void vtkModelManagerWrapper::GenerateError(cJSON* err, const std::string& errMsg, const std::string& reqId)
{
  cJSON_AddItemToObject(err, "error", cJSON_CreateString(errMsg.c_str()));
  cJSON_AddItemToObject(err, "id", cJSON_CreateString(reqId.c_str()));
}
