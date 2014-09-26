//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/remote/RemusRemoteBridge.h"
#include "smtk/bridge/remote/RemusBridgeConnection.h"

#include "smtk/options.h"

#ifdef SMTK_BUILD_CGM
#  include "smtk/bridge/cgm/Engines.h"
#endif

#include "smtk/model/Cursor.h"
#include "smtk/io/ImportJSON.h"
#include "smtk/io/ExportJSON.h"
#include "smtk/model/RemoteOperator.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ModelEntityItem.h"

#include "remus/common/MeshIOType.h"
#include "remus/proto/JobSubmission.h"

#include "cJSON.h"

namespace smtk {
  namespace bridge {
    namespace remote {

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

/// Obtain the type-id of a kernel that can be used as a worker.
RemusModelBridgeType RemusRemoteBridge::findAvailableType(
  const std::string& bridgeType)
{
  std::set<boost::shared_ptr<remus::meshtypes::MeshTypeBase> > allTypes
    = remus::common::MeshRegistrar::allRegisteredTypes();

  std::set<boost::shared_ptr<remus::meshtypes::MeshTypeBase> >::iterator it;
  for (it = allTypes.begin(); it != allTypes.end(); ++it)
    {
    RemusModelBridgeType btype = boost::dynamic_pointer_cast<RemusModelTypeBase>(*it);
    if (btype && (btype->name() == bridgeType || btype->bridgeName() == bridgeType))
      return btype;
    }

  return RemusModelBridgeType();
}

/// Return a list of the registered bridge types
smtk::model::StringList RemusRemoteBridge::availableTypeNames()
{
  smtk::model::StringList atypes;
  std::set<boost::shared_ptr<remus::meshtypes::MeshTypeBase> > allTypes
    = remus::common::MeshRegistrar::allRegisteredTypes();

  RemusModelBridgeType atype;

  std::set<boost::shared_ptr<remus::meshtypes::MeshTypeBase> >::iterator it;
  for (it = allTypes.begin(); it != allTypes.end(); ++it)
    if ((atype = boost::dynamic_pointer_cast<RemusModelTypeBase>(*it)))
      atypes.push_back(atype->name());
  return atypes;
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
  op->ensureSpecification();
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
  op->ensureSpecification();
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
    !smtk::io::ImportJSON::ofOperatorResult(res, result, op->bridge()->operatorManager()))
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

// NB: We do not invoke
// smtkImplementsModelingKernel(remus_remote,remusRemoteNoFileTypes,smtk::bridge::remote::RemusRemoteBridge);
// because each instance of the bridge may advertise different capabilities.
// Instead, when setup() is called with a pointer to a remus server, we query it to discover
// different bridges we can back and register each of them with a combined bridge name.
/*
#define smtkImplementsModelingKernel(Comp, FileTypes, Cls)

/ * Adapt create() to return a base-class pointer * /
static smtk::model::BridgePtr baseCreate() {
  return RemusRemoteBridge::create();
}

/ * Implement autoinit methods * /
void smtk_remus_remote_bridge_AutoInit_Construct()
{
  smtk::model::BRepModel::registerBridge(
    "remus_remote", / * Can't rely on bridgeName to be initialized yet * /
    fileTypes,
    baseCreate);
}

void smtk_remus_remote_bridge_AutoInit_Destruct()
{
  smtk::model::BRepModel::registerBridge(
    RemusRemoteBridge::bridgeName,
    std::vector<std::string>(),
    NULL);
}

/ **\brief Declare the component name * /
std::string RemusRemoteBridge::bridgeName("remus_remote");

/ **\brief Declare the class name * /
std::string RemusRemoteBridge::className() const { return "smtk::bridge::remote::RemusRemoteBridge"; };

/ **\brief Declare the map of operator constructors * /
smtk::model::OperatorConstructors* RemusRemoteBridge::s_operators = NULL;

/ **\brief Virtual method to allow operators to register themselves with us * /
bool RemusRemoteBridge::registerOperator(
  const std::string& opName, const char* opDescrXML,
  smtk::model::OperatorConstructor opCtor)
{
  return RemusRemoteBridge::registerStaticOperator(opName, opDescrXML, opCtor);
}

/ **\brief Allow operators to register themselves with us * /
bool RemusRemoteBridge::registerStaticOperator(
  const std::string& opName, const char* opDescrXML,
  smtk::model::OperatorConstructor opCtor)
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
    { / * unregister the operator of the given name. * /
    RemusRemoteBridge::s_operators->erase(opName);
    / * FIXME: We should ensure that no operator instances of this type are in * /
    / *        existence before allowing "unregistration" to proceed. * /
    }
  return false;
}

/ **\brief Find an operator constructor in this subclass' static list. * /
smtk::model::OperatorConstructor RemusRemoteBridge::findOperatorConstructor(
  const std::string& opName) const
{
  return this->findOperatorConstructorInternal(opName, RemusRemoteBridge::s_operators);
}

/ **\brief Find an XML description of an operator in this subclass' static list. * /
std::string RemusRemoteBridge::findOperatorXML(const std::string& opName) const
{
  return this->findOperatorXMLInternal(opName, RemusRemoteBridge::s_operators);
}

/ **\brief Called to delete registered operator map at exit. * /
void RemusRemoteBridge::cleanupOperators()
{
  delete RemusRemoteBridge::s_operators;
  RemusRemoteBridge::s_operators = NULL;
}
*/

    } // namespace remote
  } // namespace bridge
} // namespace smtk

const char* remusRemoteNoFileTypes[] = { NULL };
smtkImplementsModelingKernel(remus_remote,remusRemoteNoFileTypes,smtk::bridge::remote::RemusRemoteBridge);

smtkRegisterBridgeWithRemus("native", ,"smtk::model[native]", Native);
#if 0 // ifdef SMTK_BUILD_CGM
// FIXME: Use conditionals to enable these only when the underlying CGM library supports them:
//        At worst, this source could depend on list-cgm-engines, which provides a list.
//        At best, FindCGM could provide CMake variables to be included in smtk/options.h.in.
smtkRegisterBridgeWithRemus("cgm", smtk::bridge::cgmEngines::setDefault("ACIS"),  "smtk::model[cgm{ACIS}]", CGM_ACIS);
smtkRegisterBridgeWithRemus("cgm", smtk::bridge::cgmEngines::setDefault("cubit"), "smtk::model[cgm{Cubit}]", CGM_Cubit);
smtkRegisterBridgeWithRemus("cgm", smtk::bridge::cgmEngines::setDefault("OCC"),   "smtk::model[cgm{OpenCascade}]", CGM_OpenCascade);
smtkRegisterBridgeWithRemus("cgm", smtk::bridge::cgmEngines::setDefault("facet"), "smtk::model[cgm{Cholla}]", CGM_Cholla);
#endif
