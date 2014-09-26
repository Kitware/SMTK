#include "vtkSMModelManagerProxy.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"

#include "smtk/model/Manager.h"
#include "smtk/model/ImportJSON.h"
#include "smtk/model/ExportJSON.h"

#include "cJSON.h"

#include "vtkObjectFactory.h"
#include "vtkClientServerStream.h"
#include "vtkSMPropertyHelper.h"
#include "cmbForwardingBridge.h"

using smtk::util::UUID;
using namespace smtk::model;

vtkStandardNewMacro(vtkSMModelManagerProxy);

vtkSMModelManagerProxy::vtkSMModelManagerProxy()
{
  // This model will be mirrored (topology-only) from the server
  this->m_modelMgr = smtk::model::Manager::create();
  this->m_serverSession = NULL;
}

vtkSMModelManagerProxy::~vtkSMModelManagerProxy()
{
}

/// Print the state of this instance.
void vtkSMModelManagerProxy::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "ModelMgr: " << this->m_modelMgr.get() << "\n"; // m_modelMgr
  os << indent << "ServerSession: " << this->m_serverSession << "\n"; // m_serverSession
}

/// Return the list of bridges available on the server (not the local modelManager()'s list).
std::vector<std::string> vtkSMModelManagerProxy::bridgeNames()
{
  std::vector<std::string> resultVec;
  if (this->m_remoteBridgeNames.empty())
    {
    // Do not report our bridge names. Report those available on the server.
    std::string reqStr = "{\"jsonrpc\":\"2.0\", \"method\":\"search-bridges\", \"id\":\"1\"}";
    cJSON* result = this->jsonRPCRequest(reqStr);
    cJSON* sarr;
    if (
      !result ||
      result->type != cJSON_Object ||
      !(sarr = cJSON_GetObjectItem(result, "result")) ||
      sarr->type != cJSON_Array)
      {
      // TODO: See if result has "error" key and report it.
      if (result)
        cJSON_Delete(result);
      return std::vector<std::string>();
      }

    smtk::model::ImportJSON::getStringArrayFromJSON(sarr, resultVec);
    cJSON_Delete(result);
    this->m_remoteBridgeNames = std::set<std::string>(
      resultVec .begin(), resultVec.end());
    }
  else
    {
    resultVec = std::vector<std::string>(
      this->m_remoteBridgeNames.begin(), this->m_remoteBridgeNames.end());
    }

  return resultVec;
}

smtk::util::UUID vtkSMModelManagerProxy::beginBridgeSession(const std::string& bridgeName)
{
  if (this->m_remoteBridgeNames.find(bridgeName) == this->m_remoteBridgeNames.end())
    return smtk::util::UUID::null();

  //FIXME: Sanitize bridgeName!
  std::string reqStr =
    "{\"jsonrpc\":\"2.0\", \"method\":\"create-bridge\", \"params\":{\"bridge-name\":\"" +
    bridgeName + "\"}, \"id\":\"1\"}";
  cJSON* result = this->jsonRPCRequest(reqStr);
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
    opsObj->type != cJSON_String ||
    !opsObj->valuestring ||
    !opsObj->valuestring[0]
    )
      {
      // TODO: See if result has "error" key and report it.
      if (result)
        cJSON_Delete(result);
      return smtk::util::UUID::null();
      }

  // OK, construct a special "forwarding" bridge locally.
  cmbForwardingBridge::Ptr bridge = cmbForwardingBridge::create();
  bridge->setProxy(this);
  // The ImportJSON registers this bridge with the model manager.
  if (ImportJSON::ofRemoteBridgeSession(bridgeIdObj, bridge, this->m_modelMgr))
    {
    // Failure.
    }
  //this->m_modelMgr->registerBridgeSession(bridge);

  cJSON_Delete(result);

  UUID bridgeId = bridge->sessionId();
  this->m_remoteBridgeSessionIds[bridgeId] = bridgeName;
  return bridgeId;
}

bool vtkSMModelManagerProxy::endBridgeSession(const smtk::util::UUID& bridgeSessionId)
{
  std::map<smtk::util::UUID,std::string>::iterator it =
    this->m_remoteBridgeSessionIds.find(bridgeSessionId);
  if (it == this->m_remoteBridgeSessionIds.end())
    return false;

  // Unhook our local cmbForwardingBridge representing the remote.
  smtk::model::BridgePtr bridge = this->m_modelMgr->findBridgeSession(bridgeSessionId);
  if (bridge)
    this->m_modelMgr->unregisterBridgeSession(bridge);

  // Tell the server to unregister this session.
  // (Since the server's model manager should hold the only shared pointer
  // to the session, this should kill it.)
  std::string note =
    "{\"jsonrpc\":\"2.0\", \"method\":\"delete-bridge\" \"params\":{\"session-id\":\"" +
    bridgeSessionId.toString() + "\"}}";
  this->jsonRPCNotification(note);

  // Now remove the entry from the proxy's list of sessions.
  this->m_remoteBridgeSessionIds.erase(it);

  return true;
}

std::vector<std::string> vtkSMModelManagerProxy::supportedFileTypes(
  const std::string& bridgeName)
{
  std::vector<std::string> resultVec;
  std::string reqStr =
    "{\"jsonrpc\":\"2.0\", \"method\":\"bridge-filetypes\", "
    "\"params\":{ \"bridge-name\":\"" + bridgeName + "\"}, "
    "\"id\":\"1\"}";
  cJSON* result = this->jsonRPCRequest(reqStr);
  cJSON* sarr;
  if (
    !result ||
    result->type != cJSON_Object ||
    !(sarr = cJSON_GetObjectItem(result, "result")) ||
    sarr->type != cJSON_Array)
    {
    // TODO: See if result has "error" key and report it.
    if (result)
      cJSON_Delete(result);
    return std::vector<std::string>();
    }

  smtk::model::ImportJSON::getStringArrayFromJSON(sarr, resultVec);
  cJSON_Delete(result);
  return resultVec;
}

smtk::model::OperatorResult vtkSMModelManagerProxy::readFile(
  const std::string& fileName,
  const std::string& bridgeName)
{
  std::string actualBridgeName;
  if (bridgeName.empty())
    {
    std::set<std::string>::const_iterator bnit;
    for (
      bnit = this->m_remoteBridgeNames.begin();
      bnit != this->m_remoteBridgeNames.end() && actualBridgeName.empty();
      ++bnit)
      {
      std::vector<std::string> fileTypesForBridge = this->supportedFileTypes(*bnit);
      std::vector<std::string>::const_iterator fit;
      for (fit = fileTypesForBridge.begin(); fit != fileTypesForBridge.end(); ++fit)
        {
        std::string::size_type fEnd;
        std::string::size_type eEnd = fit->find(' ');
        std::string ext(*fit, 0, eEnd);
        std::cout << "Looking for \"" << ext << "\"\n";
        if ((fEnd = fileName.rfind(ext)) && (fileName.size() - fEnd == eEnd))
          { // matching substring is indeed at end of fileName
          actualBridgeName = *bnit;
          std::cout << "Found bridge type " << actualBridgeName << " for " << fileName << "\n";
          break;
          }
        }
      }
    }
  else
    {
    actualBridgeName = bridgeName;
    }
  std::map<smtk::util::UUID,std::string>::iterator it;
  BridgePtr bridge;
  for (
    it = this->m_remoteBridgeSessionIds.begin();
    it != this->m_remoteBridgeSessionIds.end();
    ++it)
    {
    BridgePtr tbridge = this->m_modelMgr->findBridgeSession(it->first);
    //if (tbridge && tbridge->name() == actualBridgeName)
    if (tbridge && it->second == actualBridgeName)
      {
      bridge = tbridge;
      std::cout << "Found bridge " << bridge->sessionId() << " (" << actualBridgeName << ")\n";
      break;
      }
    }
  if (!bridge)
    { // No existing bridge of that type. Create a new remote session.
    smtk::util::UUID bridgeId = this->beginBridgeSession(actualBridgeName);
    bridge = this->m_modelMgr->findBridgeSession(bridgeId);
    }
  if (!bridge)
    {
    std::cerr << "Could not find or create bridge of type \"" << actualBridgeName << "\"\n";
    return OperatorResult();
    }

  OperatorPtr readOp = bridge->op("read", this->m_modelMgr);
  if (!readOp)
    {
    std::cerr
      << "Could not create read operator for bridge"
      << " \"" << actualBridgeName << "\""
      << " (" << bridge->sessionId() << ")\n";
    return OperatorResult();
    }

  readOp->ensureSpecification();
  readOp->specification()->findFile("filename")->setValue(fileName);
  cJSON* json = cJSON_CreateObject();
  ExportJSON::forOperator(readOp, json);
  std::cout << "Found operator " << cJSON_Print(json) << ")\n";
  OperatorResult result = readOp->operate();
  json = cJSON_CreateObject();
  ExportJSON::forOperatorResult(result, json);
  std::cout << "Result " << cJSON_Print(json) << "\n";
  this->fetchWholeModel();
  this->m_modelMgr->assignDefaultNames();
  return result;
  //return readOp->operate();
}

std::vector<std::string> vtkSMModelManagerProxy::operatorNames(const smtk::util::UUID& bridgeSessionId)
{
  (void)bridgeSessionId;
  std::vector<std::string> result;
  return result;
}

smtk::model::OperatorPtr vtkSMModelManagerProxy::createOperator(
  const smtk::util::UUID& bridgeOrModelId, const std::string& opName)
{
  (void)opName;
  (void)bridgeOrModelId;
  smtk::model::OperatorPtr empty;
  return empty;
}

smtk::model::OperatorPtr vtkSMModelManagerProxy::createOperator(
  const std::string& bridgeName, const std::string& opName)
{
  (void)opName;
  (void)bridgeName;
  smtk::model::OperatorPtr empty;
  return empty;
}

smtk::model::ManagerPtr vtkSMModelManagerProxy::modelManager()
{
  return this->m_modelMgr;
}

cJSON* vtkSMModelManagerProxy::jsonRPCRequest(cJSON* req)
{
  char* reqStr = cJSON_Print(req);
  cJSON* response = this->jsonRPCRequest(reqStr);
  free(reqStr);
  return response;
}

cJSON* vtkSMModelManagerProxy::jsonRPCRequest(const std::string& req)
{
  vtkSMPropertyHelper(this, "JSONRequest").Set(req.c_str());
  this->UpdateVTKObjects();
  vtkClientServerStream stream;
  // calls "ProcessJSONRequest" function on object this->GetId() (which gets turned
  // into a pointer on the server)
  stream  << vtkClientServerStream::Invoke
          << VTKOBJECT(this)
          << "ProcessJSONRequest"
          << vtkClientServerStream::End;
  this->ExecuteStream(stream);

  this->UpdatePropertyInformation();
  // get the json response string
  std::string responseStr = vtkSMPropertyHelper(this, "JSONResponse").GetAsString();
  cJSON* response = cJSON_Parse(responseStr.c_str());
  return response;
}

void vtkSMModelManagerProxy::jsonRPCNotification(cJSON* note)
{
  char* noteStr = cJSON_Print(note);
  cJSON_Delete(note);
  this->jsonRPCNotification(noteStr);
  free(noteStr);
}

void vtkSMModelManagerProxy::jsonRPCNotification(const std::string& note)
{
  (void)note;
}

void vtkSMModelManagerProxy::fetchWholeModel()
{
  cJSON* response = this->jsonRPCRequest(
    "{\"jsonrpc\":\"2.0\", \"id\":\"1\", \"method\":\"fetch-model\"}");
  cJSON* model;
  cJSON* topo;
  //std::cout << " ----- \n\n\n" << cJSON_Print(response) << "\n ----- \n\n\n";
  if (
    response &&
    (model = cJSON_GetObjectItem(response, "result")) &&
    model->type == cJSON_Object &&
    (topo = cJSON_GetObjectItem(model, "topo")))
    ImportJSON::ofManager(topo, this->m_modelMgr);
  cJSON_Delete(response);
}
