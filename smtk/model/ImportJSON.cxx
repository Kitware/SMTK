#include "smtk/model/ImportJSON.h"

#include "smtk/model/Arrangement.h"
#include "smtk/model/DefaultBridge.h"
#include "smtk/model/Entity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/RemoteOperator.h"
#include "smtk/model/Tessellation.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Manager.h"

#include "smtk/util/AttributeReader.h"
#include "smtk/util/Logger.h"

#include "cJSON.h"

#include <string.h>

#if defined(_WIN32) && !defined(__CYGWIN__)
# define snprintf(buf, cnt, fmt, ...) _snprintf_s(buf, cnt, cnt, fmt, __VA_ARGS__)
#endif

using namespace smtk::util;

// Some cJSON helpers
namespace {
  int cJSON_GetStringValue(cJSON* valItem, std::string& val)
    {
    switch (valItem->type)
      {
    case cJSON_Number:
        {
        char valtext[64];
        snprintf(valtext, 64, "%.17g", valItem->valuedouble);
        val = valtext;
        }
      return 0;
    case cJSON_String:
      if (valItem->valuestring && valItem->valuestring[0])
        {
        val = valItem->valuestring;
        return 0;
        }
    default:
      break;
      }
    return 1;
    }
  int cJSON_GetIntegerValue(cJSON* valItem, long& val)
    {
    switch (valItem->type)
      {
    case cJSON_Number:
      val = valItem->valueint;
      return 0;
    case cJSON_String:
      if (valItem->valuestring)
        {
        char* strEnd;
        long tmp = strtol(valItem->valuestring, &strEnd, 10);
        // Only accept the conversion if the entire string is consumed:
        if (valItem->valuestring[0] && !*strEnd)
          {
          val = tmp;
          return 0;
          }
        }
    default:
      break;
      }
    return 1;
    }
  int cJSON_GetRealValue(cJSON* valItem, double& val)
    {
    switch (valItem->type)
      {
    case cJSON_Number:
      val = valItem->valuedouble;
      return 0;
    case cJSON_String:
      if (valItem->valuestring)
        {
        char* strEnd;
        double tmp = strtod(valItem->valuestring, &strEnd);
        // Only accept the conversion if the entire string is consumed:
        if (valItem->valuestring[0] && !*strEnd)
          {
          val = tmp;
          return 0;
          }
        }
    default:
      break;
      }
    return 1;
    }
  int cJSON_GetObjectIntegerValue(cJSON* node, const char* name, long& val)
    {
    cJSON* valItem = cJSON_GetObjectItem(node, name);
    if (valItem)
      {
      return cJSON_GetIntegerValue(valItem, val);
      }
    return 1;
    }
  int cJSON_GetUUIDArray(cJSON* uidRec, std::vector<smtk::util::UUID>& uids)
    {
    for (; uidRec; uidRec = uidRec->next)
      {
      if (uidRec->type == cJSON_String && uidRec->valuestring && uidRec->valuestring[0])
        {
        uids.push_back(smtk::util::UUID(uidRec->valuestring));
        }
      else
        {
        char* summary = cJSON_Print(uidRec);
        std::cerr << "Encountered non-UUID node: " << summary << ". Stopping.\n";
        free(summary);
        return 1;
        }
      }
    return 0;
    }
  int cJSON_GetObjectUUIDArray(cJSON* node, const char* name, std::vector<smtk::util::UUID>& uids)
    {
    cJSON* valItem = cJSON_GetObjectItem(node, name);
    if (valItem && valItem->type == cJSON_Array)
      {
      return cJSON_GetUUIDArray(valItem->child, uids);
      }
    return 1;
    }
  int cJSON_GetArrangement(cJSON* node, smtk::model::Arrangement& arr)
    {
    int count = 0;
    if (node->type == cJSON_Array)
      {
      cJSON* entry;
      for (entry = node->child; entry; entry = entry->next)
        {
        long eger;
        if (cJSON_GetIntegerValue(entry, eger) == 0)
          {
          arr.details().push_back(eger);
          ++count;
          }
        }
      }
    return count;
    }
  int cJSON_GetTessellationCoords(cJSON* node, smtk::model::Tessellation& tess)
    {
    if (!node)
      {
      return 0;
      }
    int count = 0;
    tess.coords().clear();
    if (node->type == cJSON_Array)
      {
      int numEntries = cJSON_GetArraySize(node);
      tess.coords().reserve(numEntries);
      cJSON* entry;
      for (entry = node->child; entry; entry = entry->next)
        {
        double coord;
        if (cJSON_GetRealValue(entry, coord) == 0)
          {
          tess.coords().push_back(coord);
          ++count;
          }
        }
      count = (count + 1) / 3; // point coordinates are 3-tuples.
      }
    return count;
    }
  int cJSON_GetTessellationConn(cJSON* node, smtk::model::Tessellation& tess)
    {
    if (!node)
      {
      return 0;
      }
    int count = 0;
    tess.conn().clear();
    if (node->type == cJSON_Array)
      {
      cJSON* entry;
      for (entry = node->child; entry; entry = entry->next)
        {
        long eger;
        if (cJSON_GetIntegerValue(entry, eger) == 0)
          {
          tess.conn().push_back(eger);
          ++count;
          }
        }
      }
    return count;
    }

  int cJSON_GetStringArray(cJSON* arrayNode, std::vector<std::string>& text)
    {
    int count = 0;
    std::string val;
    if (arrayNode->type == cJSON_Array && arrayNode->child)
      { // We expect to be passed a node of type cJSON_Array...
      for (cJSON* entry = arrayNode->child; entry; entry = entry->next)
        {
        if (cJSON_GetStringValue(entry, val) == 0)
          {
          text.push_back(val);
          ++count;
          }
        else
          {
          std::cerr
            << "Skipping node (type " << entry->type
            << ") supposedly in string array\n";
          }
        }
      }
    else if (cJSON_GetStringValue(arrayNode, val) == 0)
      { // ... however, we should also tolerate a single value.
      text.push_back(val);
      ++count;
      }
    return count;
    }

  int cJSON_GetIntegerArray(cJSON* arrayNode, std::vector<long>& values)
    {
    int count = 0;
    long val;
    if (arrayNode->type == cJSON_Array && arrayNode->child)
      { // We expect to be passed a node of type cJSON_Array...
      for (cJSON* entry = arrayNode->child; entry; entry = entry->next)
        {
        if (cJSON_GetIntegerValue(entry, val) == 0)
          {
          values.push_back(val);
          ++count;
          }
        else
          {
          std::cerr
            << "Skipping node (type " << entry->type
            << ") supposedly in integer array\n";
          }
        }
      }
    else if (cJSON_GetIntegerValue(arrayNode, val) == 0)
      { // ... however, we should also tolerate a single value.
      values.push_back(val);
      ++count;
      }
    return count;
    }

  int cJSON_GetRealArray(cJSON* arrayNode, std::vector<double>& values)
    {
    int count = 0;
    double val;
    if (arrayNode->type == cJSON_Array && arrayNode->child)
      { // We expect to be passed a node of type cJSON_Array...
      for (cJSON* entry = arrayNode->child; entry; entry = entry->next)
        {
        if (cJSON_GetRealValue(entry, val) == 0)
          {
          values.push_back(val);
          ++count;
          }
        else
          {
          std::cerr
            << "Skipping node (type " << entry->type
            << ") supposedly in double array\n";
          }
        }
      }
    else if (cJSON_GetRealValue(arrayNode, val) == 0)
      { // ... however, we should also tolerate a single value.
      values.push_back(val);
      ++count;
      }
    return count;
    }
}

namespace smtk {
  namespace model {

using smtk::util::UUID;

template<typename T>
int cJSON_GetObjectParameters(cJSON* node, T& obj, smtk::attribute::Manager* mgr, const char* attName, const char* attXML)
{
  cJSON* params = cJSON_GetObjectItem(node, attXML);
  cJSON* opspec = cJSON_GetObjectItem(node, attName);
  if (
    params && params->type == cJSON_String && params->valuestring && params->valuestring[0] &&
    opspec && opspec->type == cJSON_String && opspec->valuestring && opspec->valuestring[0])
    {
    smtk::util::Logger log;
    smtk::util::AttributeReader rdr;
    rdr.setReportDuplicateDefinitionsAsErrors(false);
    if (
      rdr.readContents(
        *mgr,
        params->valuestring, strlen(params->valuestring),
        log))
      {
      std::cerr
        << "Error. Log follows:\n---\n"
        << log.convertToString()
        << "\n---\n";
      throw std::string("Could not parse operator parameter XML.");
      }
    if (log.numberOfRecords())
      {
      std::cout << "  " << log.convertToString() << "\n";
      }

    // Now link the loaded XML to the operator instance by searching
    // the operatorManager for its name.
    obj = mgr->findAttribute(opspec->valuestring);
    return !!obj;
    }
  return 0;
}

/**\brief Create records in the \a manager given a string containing \a json data.
  *
  * The top level JSON object must be a dictionary with key "type" set to "Manager"
  * and key "topo" set to a dictionary of UUIDs with matching entries.
  */
int ImportJSON::intoModel(
  const char* json, ManagerPtr manager)
{
  int status = 0;
  if (!json || !json[0] || !manager)
    {
    std::cerr << "Invalid arguments.\n";
    return status;
    }

  cJSON* root = cJSON_Parse(json);
  if (!root)
    {
    return status;
    }
  switch(root->type)
    {
  case cJSON_Object:
    if (!root->child)
      {
      std::cerr << "Empty JSON object.\n";
      return status;
      }
    break;
  case cJSON_NULL:
  case cJSON_Number:
  case cJSON_String:
  case cJSON_Array:
  default:
    std::cerr << "Invalid toplevel JSON type (" << root->type << ").\n";
    return status;
    break;
    }

  cJSON* mtyp = cJSON_GetObjectItem(root, "type");
  if (mtyp && mtyp->type == cJSON_String && mtyp->valuestring && !strcmp(mtyp->valuestring,"Manager"))
    {
    cJSON* body = cJSON_GetObjectItem(root, "topo");
    status = ImportJSON::ofManager(body, manager);
    }

  cJSON_Delete(root);
  return status;
}

/**\brief Create records in the \a manager from a JSON dictionary, \a dict.
  *
  * The dictionary must have keys that are valid UUID strings and
  * values that describe entity, tessellation, arrangement, and/or
  * properties associated with the UUID.
  */
int ImportJSON::ofManager(
  cJSON* dict, ManagerPtr manager)
{
  if (!dict || !manager)
    {
    return 0;
    }
  int status = 1;
  for (cJSON* curChild = dict->child; curChild && status; curChild = curChild->next)
    {
    if (!curChild->string || !curChild->string[0])
      {
      std::cerr << "Empty dictionary key.\n";
      continue;
      }
    UUID uid(curChild->string);
    if (uid.isNull())
      {
      std::cerr << "Skipping malformed UUID: " << curChild->string << "\n";
      continue;
      }
    status &= ImportJSON::ofManagerEntity(uid, curChild, manager);
    status &= ImportJSON::ofManagerArrangement(uid, curChild, manager);
    status &= ImportJSON::ofManagerTessellation(uid, curChild, manager);
    status &= ImportJSON::ofManagerFloatProperties(uid, curChild, manager);
    status &= ImportJSON::ofManagerStringProperties(uid, curChild, manager);
    status &= ImportJSON::ofManagerIntegerProperties(uid, curChild, manager);
    }
  return status;
}

/**\brief Create an entity record from a JSON \a cellRec.
  *
  * The \a uid is the UUID corresponding to \a cellRec and
  * the resulting record will be inserted into \a manager.
  */
int ImportJSON::ofManagerEntity(
  const UUID& uid, cJSON* cellRec, ManagerPtr manager)
{
  long dim = 0;
  long entityFlags = 0;
  int status = 0;
  status |= cJSON_GetObjectIntegerValue(cellRec, "d", dim);
  status |= cJSON_GetObjectIntegerValue(cellRec, "e", entityFlags);
  if (status == 0)
    {
    UUIDWithEntity iter = manager->setEntityOfTypeAndDimension(uid, entityFlags, dim);
    // Ignore status from these as they need not be present:
    cJSON_GetObjectUUIDArray(cellRec, "r", iter->second.relations());
    }
  return status ? 0 : 1;
}

/**\brief Create entity arrangement records from a JSON \a dict.
  *
  * The \a uid is the UUID corresponding to \a dict and
  * the resulting record will be inserted into \a manager.
  */
int ImportJSON::ofManagerArrangement(
  const UUID& uid, cJSON* dict, ManagerPtr manager)
{
  cJSON* arrNode = cJSON_GetObjectItem(dict, "a");
  if (!arrNode)
    { // Missing arrangement is not an error.
    return 1;
    }
  if (arrNode->type != cJSON_Object)
    { // An improper arrangement is an error.
    return 0;
    }

  for (int i = 0; i < smtk::model::KINDS_OF_ARRANGEMENTS; ++i)
    {
    ArrangementKind k = static_cast<ArrangementKind>(i);
    std::string abbr = AbbreviationForArrangementKind(k);
    cJSON* arrangements = cJSON_GetObjectItem(arrNode, abbr.c_str());
    if (arrangements && arrangements->type == cJSON_Array)
      {
      // First, erase any pre-existing arrangements to avoid duplicates.
      manager->arrangementsOfKindForEntity(uid, k).clear();
      // Now insert arrangements from the JSON object
      for (cJSON* arr = arrangements->child; arr; arr = arr->next)
        {
        if (arr->type == cJSON_Array)
          {
          Arrangement a;
          if (cJSON_GetArrangement(arr, a) > 0)
            {
            manager->arrangeEntity(uid, k, a);
            }
          }
        }
      }
    }
  return 1;
}

/**\brief Create an entity tessellation record from a JSON \a dict.
  *
  * The \a uid is the UUID corresponding to \a dict and
  * the resulting record will be inserted into \a manager.
  */
int ImportJSON::ofManagerTessellation(
  const UUID& uid, cJSON* dict, ManagerPtr manager)
{
  cJSON* tessNode = cJSON_GetObjectItem(dict, "t");
  if (!tessNode)
    { // Missing tessellation is not an error.
    return 1;
    }
  if (tessNode->type != cJSON_Object)
    { // An improper tessellation is an error.
    return 0;
    }
  // Now extract graphics primitives from the JSON data.
  // We should fetch the metadata->formatVersion and verify it,
  // but I don't think it makes any difference to the fields
  // we rely on... yet.
  UUIDsToTessellations::iterator tessIt = manager->tessellations().find(uid);
  if (tessIt == manager->tessellations().end())
    {
    Tessellation blank;
    tessIt = manager->tessellations().insert(
      std::pair<UUID,Tessellation>(uid, blank)).first;
    }
  int numVerts = cJSON_GetTessellationCoords(
    cJSON_GetObjectItem(tessNode, "vertices"), tessIt->second);
  int numPrims = cJSON_GetTessellationConn(
    cJSON_GetObjectItem(tessNode, "faces"), tessIt->second);
  (void)numVerts;
  (void)numPrims;
  //std::cout << uid << " has " << numVerts << " verts " << numPrims << " prims\n";
  return 1;
}

/**\brief Create entity floating-point-property records from a JSON \a dict.
  *
  * The \a uid is the UUID corresponding to \a dict and
  * the resulting record will be inserted into \a manager.
  */
int ImportJSON::ofManagerFloatProperties(const smtk::util::UUID& uid, cJSON* dict, ManagerPtr manager)
{
  int status = 0;
  cJSON* floatNode = cJSON_GetObjectItem(dict, "f");
  if (!floatNode)
    { // Missing floating-point property map is not an error.
    return 1;
    }
  for (cJSON* floatProp = floatNode->child; floatProp; floatProp = floatProp->next)
    {
    if (!floatProp->string || !floatProp->string[0])
      { // skip un-named property arrays.
      continue;
      }
    FloatList propVal;
    cJSON_GetRealArray(floatProp, propVal);
    manager->setFloatProperty(uid, floatProp->string, propVal);
    }
  return status ? 0 : 1;
}

/**\brief Create entity string-property records from a JSON \a dict.
  *
  * The \a uid is the UUID corresponding to \a dict and
  * the resulting record will be inserted into \a manager.
  */
int ImportJSON::ofManagerStringProperties(const smtk::util::UUID& uid, cJSON* dict, ManagerPtr manager)
{
  int status = 0;
  cJSON* stringNode = cJSON_GetObjectItem(dict, "s");
  if (!stringNode)
    { // Missing floating-point property map is not an error.
    return 1;
    }
  for (cJSON* stringProp = stringNode->child; stringProp; stringProp = stringProp->next)
    {
    if (!stringProp->string || !stringProp->string[0])
      { // skip un-named property arrays.
      continue;
      }
    StringList propVal;
    cJSON_GetStringArray(stringProp, propVal);
    manager->setStringProperty(uid, stringProp->string, propVal);
    }
  return status ? 0 : 1;
}

/**\brief Create entity integer-property records from a JSON \a dict.
  *
  * The \a uid is the UUID corresponding to \a dict and
  * the resulting record will be inserted into \a manager.
  */
int ImportJSON::ofManagerIntegerProperties(const smtk::util::UUID& uid, cJSON* dict, ManagerPtr manager)
{
  int status = 0;
  cJSON* integerNode = cJSON_GetObjectItem(dict, "i");
  if (!integerNode)
    { // Missing floating-point property map is not an error.
    return 1;
    }
  for (cJSON* integerProp = integerNode->child; integerProp; integerProp = integerProp->next)
    {
    if (!integerProp->string || !integerProp->string[0])
      { // skip un-named property arrays.
      continue;
      }
    IntegerList propVal;
    cJSON_GetIntegerArray(integerProp, propVal);
    manager->setIntegerProperty(uid, integerProp->string, propVal);
    }
  return status ? 0 : 1;
}

/**\brief Import JSON holding a bridge session into a local bridge.
  *
  * You are responsible for providing the \a destBridge instance
  * into which the \a node's session will be placed.
  * You must also provide a valid model manager, and \a destBridge
  * will be registered with \a context after its session ID has
  * been assigned.
  * The \a destBridge must be of a proper type for your application
  * (i.e., be able to forward requests for data and operations).
  */
int ImportJSON::ofRemoteBridgeSession(cJSON* node, DefaultBridgePtr destBridge, ManagerPtr context)
{
  int status = 0;
  cJSON* opsObj;
  cJSON* nameObj;
  if (
    !node ||
    node->type != cJSON_Object ||
    // Does the node have a valid bridge session ID?
    !node->string ||
    !node->string[0] ||
    // Does the node have fields "name" and "ops" (for "operators") of type String?
    !(nameObj = cJSON_GetObjectItem(node, "name")) ||
    nameObj->type != cJSON_String ||
    !nameObj->valuestring ||
    !nameObj->valuestring[0] ||
    !(opsObj = cJSON_GetObjectItem(node, "ops")) ||
    opsObj->type != cJSON_String ||
    !opsObj->valuestring ||
    !opsObj->valuestring[0])
    return status;

  destBridge->backsRemoteBridge(
    nameObj->valuestring, smtk::util::UUID(node->string));

  // Import the XML definitions of the serialized bridge session
  // into the destination bridge's operatorManager():
  smtk::util::Logger log;
  smtk::util::AttributeReader rdr;
  rdr.setReportDuplicateDefinitionsAsErrors(false);
  if (
    rdr.readContents(
      *destBridge->operatorManager(),
      opsObj->valuestring, strlen(opsObj->valuestring),
      log))
    {
    std::cerr
      << "Error. Log follows:\n---\n"
      << log.convertToString()
      << "\n---\n";
    throw std::string("Could not parse operator XML.");
    }
  if (log.numberOfRecords())
    {
    std::cout << "  " << log.convertToString() << "\n";
    }

  // Register the bridge session with the model manager:
  context->registerBridgeSession(destBridge);

  // Now register the RemoteOperator constructor with each
  // operator in the bridge session.
  // NB: This registers the constructor with the entire
  //     bridge class, not just the destBridge instance.
  //     If destBridge is a DefaultBridge (and not a subclass
  //     of it), then be aware that this may override non-RemoteOperator
  //     constructors with RemoteOperator constructors for operators
  //     of the same name.
  StringList opNames = destBridge->operatorNames();
  for (StringList::iterator it = opNames.begin(); it != opNames.end(); ++it)
    {
    destBridge->registerOperator(*it, NULL, RemoteOperator::baseCreate);
    }
  return status;
}

/**\brief Import JSON for an operator into an Operator instance.
  *
  * **Important**: Unlike other JSON import methods, this method
  * creates a new instance of an Operator subclass, storing the result
  * into \a op.
  *
  * If the JSON \a node contains a "sessionId" property,
  * the storage manager \a context is searched for a Bridge with the
  * matching UUID. If no matching Bridge exists, then
  * a RemoteOperator is created on the default Bridge and
  * its session ID set to the corresponding value.
  * If no "sessionId" is present in \a node, then the method returns
  * 0 (failure) and \a op is unchanged.
  *
  * If the JSON \a node has no "name" property (or has a
  * name unknown to the Bridge), then the method returns 0 (failure).
  *
  * Finally, parameter values stored in \a node's "param"
  * string (as XML) are read into the operator's attribute manager.
  */
int ImportJSON::ofOperator(cJSON* node, OperatorPtr& op, ManagerPtr context)
{
  cJSON* pnode;

  std::string osess;
  pnode = cJSON_GetObjectItem(node, "sessionId");
  smtk::util::UUID sessionId;
  if (
    !pnode ||
    cJSON_GetStringValue(pnode, osess) ||
    osess.empty() ||
    (sessionId = smtk::util::UUID(osess)).isNull())
    return 0;

  BridgePtr bridge;
  DefaultBridge::Ptr defBridge;
  if (context)
    {
    bridge = context->findBridgeSession(sessionId);
    defBridge = smtk::dynamic_pointer_cast<DefaultBridge>(bridge);
    }

  std::string oname;
  pnode = cJSON_GetObjectItem(node, "name");
  if (!pnode || cJSON_GetStringValue(pnode, oname))
    return 0;

  if (defBridge) defBridge->setImportingOperators(true);
  op = bridge->op(oname, context);
  if (defBridge) defBridge->setImportingOperators(false);
  if (!op)
    return 0;

  // If the operator has a specification, use it.
  // It is not an error to pass an unspecified operator.
  OperatorSpecification spec;
  if (
    cJSON_GetObjectParameters(
      node, spec, op->bridge()->operatorManager(), "spec", "specXML"))
    {
    op->setSpecification(spec);
    }
  return 1;
}

int ImportJSON::ofOperatorResult(cJSON* node, OperatorResult& resOut, smtk::attribute::Manager* opMgr)
{
  return cJSON_GetObjectParameters(node, resOut, opMgr, "result", "resultXML");
}

int ImportJSON::ofDanglingEntities(cJSON* node, ManagerPtr context)
{
  if (!node || !context)
    return 0;
  cJSON* danglers = cJSON_GetObjectItem(node, "danglingEntities");
  if (!danglers || danglers->type != cJSON_Object)
    return 0;

  cJSON* sessId = cJSON_GetObjectItem(danglers, "sessionId");
  if (!sessId || sessId->type != cJSON_String || !sessId->valuestring || !sessId->valuestring[0])
    return 0;

  smtk::util::UUID bridgeSessionId(sessId->valuestring);
  if (bridgeSessionId.isNull())
    return 0;
  BridgePtr bridge = context->findBridgeSession(bridgeSessionId);
  if (!bridge)
    return 0;

  cJSON* darray = cJSON_GetObjectItem(danglers, "entities");
  if (!darray || darray->type != cJSON_Object || !darray->child)
    return 0;

  cJSON* entry;
  for (entry = darray->child; entry; entry = entry->next)
    {
    if (!entry->string || !entry->string[0] || entry->type != cJSON_Number)
      continue;
    smtk::util::UUID entityId(entry->string);
    if (entityId.isNull())
      continue;
    smtk::model::Cursor c(context, entityId);
    bridge->declareDanglingEntity(c, static_cast<BridgedInfoBits>(entry->valueint));
    }

  return 1;
}

int ImportJSON::getUUIDArrayFromJSON(cJSON* uidRec, std::vector<smtk::util::UUID>& uids)
{
  return cJSON_GetUUIDArray(uidRec, uids);
}

int ImportJSON::getStringArrayFromJSON(cJSON* arrayNode, std::vector<std::string>& text)
{
  return cJSON_GetStringArray(arrayNode, text);
}

int ImportJSON::getIntegerArrayFromJSON(cJSON* arrayNode, std::vector<long>& values)
{
  return cJSON_GetIntegerArray(arrayNode, values);
}

int ImportJSON::getRealArrayFromJSON(cJSON* arrayNode, std::vector<double>& values)
{
  return cJSON_GetRealArray(arrayNode, values);
}

  }
}
