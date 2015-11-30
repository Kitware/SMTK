//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/io/ImportJSON.h"

#include "smtk/model/Arrangement.h"
#include "smtk/model/SessionRegistrar.h"
#include "smtk/model/SessionIOJSON.h"
#include "smtk/model/DefaultSession.h"
#include "smtk/model/Entity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/RemoteOperator.h"
#include "smtk/model/StringData.h"
#include "smtk/model/Tessellation.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/System.h"

#include "smtk/mesh/Manager.h"
#include "smtk/mesh/Collection.h"
#include "smtk/mesh/json/Interface.h"
#include "smtk/mesh/moab/Interface.h"

#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"
#include "smtk/io/ImportMesh.h"

#include "cJSON.h"

#include <stdio.h>
#include <string.h>

#if defined(_WIN32) && !defined(__CYGWIN__)
# define snprintf(buf, cnt, fmt, ...) _snprintf_s(buf, cnt, cnt, fmt, __VA_ARGS__)
#endif

using namespace smtk::io;
using namespace smtk::common;
using namespace smtk::model;

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
  int cJSON_GetUUIDArray(cJSON* uidRec, std::vector<smtk::common::UUID>& uids)
    {
    for (; uidRec; uidRec = uidRec->next)
      {
      if (uidRec->type == cJSON_String && uidRec->valuestring && uidRec->valuestring[0])
        {
        uids.push_back(smtk::common::UUID(uidRec->valuestring));
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
  int cJSON_GetObjectUUIDArray(cJSON* node, const char* name, std::vector<smtk::common::UUID>& uids)
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
  namespace io {

template<typename T>
int cJSON_GetObjectParameters(cJSON* node, T& obj, smtk::attribute::System* sys, const char* attName, const char* attXML)
{
  cJSON* params = cJSON_GetObjectItem(node, attXML);
  cJSON* opspec = cJSON_GetObjectItem(node, attName);
  if (
    params && params->type == cJSON_String && params->valuestring && params->valuestring[0] &&
    opspec && opspec->type == cJSON_String && opspec->valuestring && opspec->valuestring[0])
    {
    smtk::io::Logger log;
    smtk::io::AttributeReader rdr;
    rdr.setReportDuplicateDefinitionsAsErrors(false);
    if (
      rdr.readContents(
        *sys,
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
    // the operatorSystem for its name.
    obj = sys->findAttribute(opspec->valuestring);
    return !!obj;
    }
  return 0;
}

/**\brief Create records in the \a manager given a string containing \a json data.
  *
  * The top level JSON object must be a dictionary with key "type" set to "Manager"
  * and key "topo" set to a dictionary of UUIDs with matching entries.
  */
int ImportJSON::intoModelManager(
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
    status &= ImportJSON::ofManagerAnalysis(uid, curChild, manager);
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

/**\brief Create an entity analysis mesh record from a JSON \a dict.
  *
  * The \a uid is the UUID corresponding to \a dict and
  * the resulting record will be inserted into \a manager.
  */
int ImportJSON::ofManagerAnalysis(
  const UUID& uid, cJSON* dict, ManagerPtr manager)
{
  cJSON* meshNode = cJSON_GetObjectItem(dict, "m");
  if (!meshNode)
    { // Missing tessellation is not an error.
    return 1;
    }
  if (meshNode->type != cJSON_Object)
    { // An improper tessellation is an error.
    return 0;
    }
  // Now extract graphics primitives from the JSON data.
  // We should fetch the metadata->formatVersion and verify it,
  // but I don't think it makes any difference to the fields
  // we rely on... yet.
  UUIDsToTessellations::iterator meshIt = manager->analysisMesh().find(uid);
  if (meshIt == manager->analysisMesh().end())
    {
    Tessellation blank;
    meshIt = manager->analysisMesh().insert(
      std::pair<UUID,Tessellation>(uid, blank)).first;
    }
  int numVerts = cJSON_GetTessellationCoords(
    cJSON_GetObjectItem(meshNode, "vertices"), meshIt->second);
  int numPrims = cJSON_GetTessellationConn(
    cJSON_GetObjectItem(meshNode, "faces"), meshIt->second);
  (void)numVerts;
  (void)numPrims;
  return 1;
}

/**\brief Create entity floating-point-property records from a JSON \a dict.
  *
  * The \a uid is the UUID corresponding to \a dict and
  * the resulting record will be inserted into \a manager.
  */
int ImportJSON::ofManagerFloatProperties(const smtk::common::UUID& uid, cJSON* dict, ManagerPtr manager)
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
int ImportJSON::ofManagerStringProperties(const smtk::common::UUID& uid, cJSON* dict, ManagerPtr manager)
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
int ImportJSON::ofManagerIntegerProperties(const smtk::common::UUID& uid, cJSON* dict, ManagerPtr manager)
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

/**\brief Import JSON holding a session into a local session.
  *
  * The session described by \a node will be mirrored by the
  * \a destSession you specify.
  *
  * You are responsible for providing the \a destSession instance
  * into which the \a node's session will be placed.
  * You must also provide a valid model manager, and \a destSession
  * will be registered with \a context after its session ID has
  * been assigned.
  * The \a destSession must be of a proper type for your application
  * (i.e., be able to forward requests for data and operations).
  */
int ImportJSON::ofRemoteSession(cJSON* node, DefaultSessionPtr destSession, ManagerPtr context)
{
  int status = 0;
  cJSON* opsObj;
  cJSON* nameObj;
  if (
    !node ||
    node->type != cJSON_Object ||
    // Does the node have a valid session ID?
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

  destSession->backsRemoteSession(
    nameObj->valuestring, smtk::common::UUID(node->string));

  // Import the XML definitions of the serialized session
  // into the destination session's operatorSystem():
  smtk::io::Logger log;
  smtk::io::AttributeReader rdr;
  rdr.setReportDuplicateDefinitionsAsErrors(false);
  if (
    rdr.readContents(
      *destSession->operatorSystem(),
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

  // Register the session with the model manager:
  context->registerSession(destSession);

  // Now register the RemoteOperator constructor with each
  // operator in the session.
  // NB: This registers the constructor with the entire
  //     session class, not just the destSession instance.
  //     If destSession is a DefaultSession (and not a subclass
  //     of it), then be aware that this may override non-RemoteOperator
  //     constructors with RemoteOperator constructors for operators
  //     of the same name.
  StringList opNames = destSession->operatorNames();
  for (StringList::iterator it = opNames.begin(); it != opNames.end(); ++it)
    {
    destSession->registerOperator(*it, NULL, RemoteOperator::baseCreate);
    }

  // Import additional state if the session can accept it.
  // Note that this is tricky because createIODelegate is
  // being called on a *remote* session (i.e., subclass of
  // DefaultSession, not Session) while the JSON is created
  // by a delegate obtained by calling createIODelegate
  // on a *local* session (i.e., likely a direct subclass
  // of Session).
  SessionIOJSONPtr delegate =
    smtk::dynamic_pointer_cast<SessionIOJSON>(
      destSession->createIODelegate("json"));
  if (delegate)
    {
    delegate->importJSON(context, node);
    }
  return status;
}

/**\brief Create a local session and import JSON from a session.
  *
  * The session described by \a node will be mirrored by a newly-created
  * session (whose type is specified by \a node) and attached to the
  * model manager \a context you specify.
  *
  * You must provide a valid model manager, \a context, to which the
  * restored session will be registered.
  * Note that \a context must *already* contain the SMTK entity records
  * for the session!
  * In particular, Model entries in \a context will be used to
  * determine the URLs for the modeling kernel's native representations.
  *
  * Note that it may not be possible to create an instance of the
  * given session type -- either because support has not been compiled
  * into SMTK or because the URL is locally inaccessible.
  * (An example of latter is calling this method on a machine different
  * from the source machine so that the original files are unavailable.)
  * In these cases, 0 will be returned.
  * 1 will be returned on success.
  *
  * Note that it is up to individual session subclasses to
  * provide mechanisms for loading kernel-native models while
  * preserving UUIDs. The expected design pattern is for each session
  * to provide a SessionIOJSON delegate class that reads kernel-specific
  * keys in the session. These keys will specify the list
  * of models associated with the session. The delegate can
  * then query the \a context for URLs and either directly load
  * the URL or use the "read" operator for the session to load the URL.
  * Since "read" operators usually insert new entries into \a context,
  * special care must be taken to avoid that behavior when importing
  * a session.
  */
int ImportJSON::ofLocalSession(cJSON* node, ManagerPtr context)
{
  int status = 0;
  cJSON* opsObj;
  cJSON* nameObj;
  if (
    !node ||
    node->type != cJSON_Object ||
    // Does the node have a valid session ID?
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

  SessionRef sref(context, smtk::common::UUID(node->string));
  sref =
    context->createSession(
      nameObj->valuestring, sref);
  if (!sref.isValid())
    return status;

  // Ignore the XML definitions of the serialized session;
  // recreating the session will recreate the attribute definitions.
  (void)opsObj;

  // Import additional state.
  // Sessions may use this to determine which model entities
  // in the \a context are associated with the new session
  // and load the corresponding native-kernel model files.
  SessionIOJSONPtr delegate =
    smtk::dynamic_pointer_cast<SessionIOJSON>(
      sref.session()->createIODelegate("json"));
  if (delegate)
    {
    status = delegate->importJSON(context, node);
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
  * the storage manager \a context is searched for a Session with the
  * matching UUID. If no matching Session exists, then
  * a RemoteOperator is created on the default Session and
  * its session ID set to the corresponding value.
  * If no "sessionId" is present in \a node, then the method returns
  * 0 (failure) and \a op is unchanged.
  *
  * If the JSON \a node has no "name" property (or has a
  * name unknown to the Session), then the method returns 0 (failure).
  *
  * Finally, parameter values stored in \a node's "param"
  * string (as XML) are read into the operator's attribute manager.
  */
int ImportJSON::ofOperator(cJSON* node, OperatorPtr& op, ManagerPtr context)
{
  cJSON* pnode;

  std::string osess;
  pnode = cJSON_GetObjectItem(node, "sessionId");
  smtk::common::UUID sessionId;
  if (
    !pnode ||
    cJSON_GetStringValue(pnode, osess) ||
    osess.empty() ||
    (sessionId = smtk::common::UUID(osess)).isNull())
    return 0;

  SessionPtr session;
  DefaultSession::Ptr defSession;
  if (context)
    {
    session = SessionRef(context, sessionId).session();
    defSession = smtk::dynamic_pointer_cast<DefaultSession>(session);
    }

  std::string oname;
  pnode = cJSON_GetObjectItem(node, "name");
  if (!pnode || cJSON_GetStringValue(pnode, oname))
    return 0;

  op = session->op(oname);
  if (!op)
    return 0;

  // If the operator has a specification, use it.
  // It is not an error to pass an unspecified operator.
  OperatorSpecification spec;
  if (
    cJSON_GetObjectParameters(
      node, spec, op->session()->operatorSystem(), "spec", "specXML"))
    {
    op->setSpecification(spec);
    }
  return 1;
}

int ImportJSON::ofOperatorResult(cJSON* node, OperatorResult& resOut, smtk::model::RemoteOperatorPtr op)
{
  smtk::attribute::System* opSys = op->session()->operatorSystem();
  // Deserialize the OperatorResult into \a resOut:
  int status = cJSON_GetObjectParameters(node, resOut, opSys, "result", "resultXML");

  // Remove entities that the operator result reports as expunged:
  smtk::attribute::ModelEntityItemPtr expunged = resOut->findModelEntity("expunged");
  std::size_t num = expunged->numberOfValues();
  for (std::size_t i = 0; i < num; ++i)
    {
    // if this is a removed model, the children of the model should also be removed.
    if(expunged->value(i).isModel())
      {
      expunged->value(i).manager()->eraseModel(expunged->value(i).as<smtk::model::Model>());
      }
    else
      {
      expunged->value(i).manager()->erase(expunged->value(i));
      }
    }

  // Deserialize the relevant transcribed entities into the
  // remote operator's model manager:
  smtk::model::ManagerPtr mgr = op->manager();
  cJSON* records = cJSON_GetObjectItem(node, "records");
  cJSON* mesh_records = cJSON_GetObjectItem(node, "mesh_records");
  if (mgr)
    {
    if(records)
      {
      for (cJSON* c = records->child; c; c = c->next)
        mgr->erase(smtk::common::UUID(c->string));
      status = ImportJSON::ofManager(records, mgr);
      }

    if(mesh_records)
      {
      status &= ImportJSON::ofMeshesOfModel(mesh_records, mgr, true);
      }
    }
  return status;
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

  smtk::common::UUID sessionId(sessId->valuestring);
  if (sessionId.isNull())
    return 0;
  SessionPtr session = SessionRef(context, sessionId).session();
  if (!session)
    return 0;

  cJSON* darray = cJSON_GetObjectItem(danglers, "entities");
  if (!darray || darray->type != cJSON_Object || !darray->child)
    return 0;

  cJSON* entry;
  for (entry = darray->child; entry; entry = entry->next)
    {
    if (!entry->string || !entry->string[0] || entry->type != cJSON_Number)
      continue;
    smtk::common::UUID entityId(entry->string);
    if (entityId.isNull())
      continue;
    smtk::model::EntityRef c(context, entityId);
    session->declareDanglingEntity(c, static_cast<SessionInfoBits>(entry->valueint));
    }

  return 1;
}

/**\brief Append all of the entries in \a jsonStr (a string containing a JSON array of arrays) to the \a log.
  *
  * See the other variant for details.
  */
int ImportJSON::ofLog(const char* jsonStr, smtk::io::Logger& log)
{
  cJSON* json = cJSON_Parse(jsonStr);
  int stat = ImportJSON::ofLog(json, log);
  cJSON_Delete(json);
  return stat;
}

/**\brief Append all of the entries in \a logrecordarray (an array of arrays) to the \a log.
  *
  * This returns the number of records (whether or not they were actually
  * converted to log entries) or -1 if \a logrecordarray is invalid.
  * When the logrecordarray is valid but some individual entries were
  * not formatted as expected, then the return value will be higher than
  * the number of records actually appended to the \a log.
  */
int ImportJSON::ofLog(cJSON* logrecordarray, smtk::io::Logger& log)
{
  if (!logrecordarray || logrecordarray->type != cJSON_Array)
    return -1;

  int numberOfRecords = 0;
  cJSON* entry;
  for (entry = logrecordarray->child; entry; entry = entry->next)
    {
    cJSON* severity;
    cJSON* msg;
    cJSON* file;
    cJSON* line;
    ++numberOfRecords;
    // Every entry must be an array with at least 2 entries:
    // an integer (severity) and a non-empty string (log message).
    // Entries may optionally contain a filename and line number.
    if (
      entry && entry->type == cJSON_Array &&
      (severity = entry->child) && (severity->type == cJSON_Number) &&
      (msg = severity->next) && (msg->type == cJSON_String) &&
      msg->valuestring && msg->valuestring[0])
      {
      file = msg->next;
      line = file ? file->next : NULL;
      if (file && file->type == cJSON_String && file->valuestring && file->valuestring[0])
        if (line && line->type == cJSON_Number)
          log.addRecord(
            static_cast<smtk::io::Logger::Severity>(severity->valueint),
            msg->valuestring,
            file->valuestring,
            static_cast<unsigned int>(line->valueint));
        else
          log.addRecord(
            static_cast<smtk::io::Logger::Severity>(severity->valueint),
            msg->valuestring,
            file->valuestring);
      else
        log.addRecord(
          static_cast<smtk::io::Logger::Severity>(severity->valueint),
          msg->valuestring);
      }
    }
  return numberOfRecords;
}

/**\brief Import all the smtk::mesh::Collections associated with a given smtk::model.
  *
  * All collections listed in \a node are imported in, and added to the mesh
  * manager that is owned by the model \a meshMgr.
  *
  * Returns a status value of 1 when everything has been loading in properly
  *
  */
int ImportJSON::ofMeshesOfModel(cJSON* node,
                                smtk::model::ManagerPtr modelMgr,
                                bool updateExisting)

{
  int status = 1;
  if (!node || !modelMgr)
    {
    return 0;
    }

  cJSON* collections;
  if (node->type != cJSON_Object ||
      // Does the node have fields "mesh_collections"
      !(collections = cJSON_GetObjectItem(node, "mesh_collections")) )
    { //not having meshes is not a failure
    return status;
    }

  smtk::mesh::ManagerPtr meshMgr = modelMgr->meshes();
  for (cJSON* child = collections->child; child && status; child = child->next)
    {
    if (!child->string || !child->string[0])
      {
      std::cerr << "Empty dictionary key.\n";
      continue;
      }

    //get the uuid to use for this collection
    UUID uid(child->string);
    if (uid.isNull())
      {
      std::cerr << "Skipping malformed UUID: " << child->string << "\n";
      continue;
      }

    //first verify the collection doesn't already exist
    if (smtk::mesh::CollectionPtr existingC = meshMgr->collection(uid))
      {
      if(updateExisting)
        {
        // Import everything in a json string into the existing collection?, need to clear first?
        // smtk::mesh::json::import(child, existingC);
        // update properties, if any, to the existing collection
        status &= ImportJSON::ofMeshProperties(child, existingC);
        }
      else
        {
        std::cerr << "Importing a mesh collection that already exists: " << child->string << "\n";
        }
      continue;
      }
    //assoicated model uuid of the collection
    smtk::common::UUID associatedModelId;
    if(cJSON* modelIdNode = cJSON_GetObjectItem(child, "associatedModel"))
      {
      std::string modelIdVal;
      cJSON_GetStringValue(modelIdNode, modelIdVal);
      associatedModelId = UUID(modelIdVal);
      }
    //get the location and type nodes from json
    cJSON* fLocationNode = cJSON_GetObjectItem(child, "location");
    cJSON* fTypeNode = cJSON_GetObjectItem(child, "type");
    std::string collectionTypeName;
    cJSON_GetStringValue(fTypeNode, collectionTypeName);

    //todo codify "moab" and "json" string types as proper types
    const bool isValidMoab = collectionTypeName == std::string("moab") &&
                             fLocationNode;

    smtk::mesh::CollectionPtr importedCollection;
    if(isValidMoab)
      {
      //get the file_path from json
      std::string file_path;
      cJSON_GetStringValue(fLocationNode, file_path);
      importedCollection = smtk::io::ImportMesh::entireFile(file_path, meshMgr);
      }

    //wasnt moab, or failed to load as moab
    if(!isValidMoab || !importedCollection)
      {
      importedCollection = smtk::io::ImportMesh::entireJSON(child, meshMgr);
      }

    if(importedCollection)
      {
      //Transfer ownership of the interface over to this new collection
      //done so that we get the correct uuid for the collection
      smtk::mesh::CollectionPtr collection =
            meshMgr->makeCollection(uid, importedCollection->interface());

      //remove the old collection, as its interface is now owned by the new
      //collection
      meshMgr->removeCollection(importedCollection);
      importedCollection.reset();

      //set the name back to the collection
      cJSON* collecNameNode = cJSON_GetObjectItem(child, "name");
      std::string collectionName;
      cJSON_GetStringValue(collecNameNode, collectionName);
      collection->name(collectionName);

      //set the collections model manager so that we can do model based
      //queries properly
      collection->setModelManager( modelMgr );

      if(!associatedModelId.isNull())
        {
        collection->associateToModel(associatedModelId);
        }
      //write properties to the new collection
      status &= ImportJSON::ofMeshProperties(child, collection);
      }
    else
      {
      std::cerr << "Unable to import the collection. \n";
      continue;
      }
    }
  return status;
}

/**\brief Import all mesh properties of an smtk::mesh::Collection.
  *
  * All mesh properties in the json \a node for collection are imported in,
  * and added to the mesh \a collection.
  *
  */
int ImportJSON::ofMeshProperties(cJSON* node,
                                smtk::mesh::CollectionPtr collection)
{
  cJSON* jsonProperties = cJSON_GetObjectItem(node, "properties");
  if(!jsonProperties)
    return 1;

  // iterate through all mesh properties records
  for (cJSON* meshEntry = jsonProperties->child; meshEntry; meshEntry = meshEntry->next)
    {
    smtk::mesh::HandleRange hrange = smtk::mesh::from_json(meshEntry);
    smtk::mesh::MeshSet mesh = smtk::mesh::MeshSet(
      collection, collection->interface()->getRoot(), hrange);

    // float properties
    cJSON* floatNode = cJSON_GetObjectItem(meshEntry, "f");
    if (floatNode)
      {
      for (cJSON* floatProp = floatNode->child; floatProp; floatProp = floatProp->next)
        {
        if (!floatProp->string || !floatProp->string[0])
          { // skip un-named property arrays.
          continue;
          }
        FloatList propVal;
        cJSON_GetRealArray(floatProp, propVal);
        collection->setFloatProperty(mesh, floatProp->string, propVal);
        }
      }
    // string properties
    cJSON* stringNode = cJSON_GetObjectItem(meshEntry, "s");
    if (stringNode)
      {
      for (cJSON* stringProp = stringNode->child; stringProp; stringProp = stringProp->next)
        {
        if (!stringProp->string || !stringProp->string[0])
          { // skip un-named property arrays.
          continue;
          }
        StringList propVal;
        cJSON_GetStringArray(stringProp, propVal);
        collection->setStringProperty(mesh, stringProp->string, propVal);
        }
      }
    // integer properties
    cJSON* integerNode = cJSON_GetObjectItem(meshEntry, "i");
    if (integerNode)
      {
      for (cJSON* intProp = integerNode->child; intProp; intProp = intProp->next)
        {
        if (!intProp->string || !intProp->string[0])
          { // skip un-named property arrays.
          continue;
          }
        IntegerList propVal;
        cJSON_GetIntegerArray(intProp, propVal);
        collection->setIntegerProperty(mesh, intProp->string, propVal);
        }
      }
    }

  return 1;
}

std::string ImportJSON::sessionNameFromTagData(cJSON* tagData)
{
  std::ostringstream bname;
  bname << "smtk::model[";
  std::string kernel;
  cJSON* kernelJSON = cJSON_GetObjectItem(tagData, "kernel");
  if (kernelJSON)
    cJSON_GetStringValue(kernelJSON, kernel);
  bname << (kernel.empty() ? "native" : kernel);
  cJSON* enginesJSON;
  if ((enginesJSON = cJSON_GetObjectItem(tagData, "engines")))
    {
    StringList engines;
    ImportJSON::getStringArrayFromJSON(enginesJSON, engines);
    StringList::const_iterator it = engines.begin();
    if (it != engines.end())
      {
      bname << "{" << *it;
      for (++it; it != engines.end(); ++it)
        bname << "," << *it;
      bname << "}";
      }
    }
  bname << "]";
  std::string server;
  cJSON* serverJSON = cJSON_GetObjectItem(tagData, "server");
  if (serverJSON)
    cJSON_GetStringValue(serverJSON, server);
  if (!server.empty())
    bname << "@" << server;
  return bname.str();
}

smtk::model::StringList ImportJSON::sessionFileTypesFromTagData(cJSON* tagData)
{
  StringList fileTypes;
  cJSON* fileTypesJSON;
  if ((fileTypesJSON = cJSON_GetObjectItem(tagData, "fileTypes")))
    ImportJSON::getStringArrayFromJSON(fileTypesJSON, fileTypes);
  return fileTypes;
}

int ImportJSON::getUUIDArrayFromJSON(cJSON* uidRec, std::vector<smtk::common::UUID>& uids)
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
