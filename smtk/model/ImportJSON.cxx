#include "smtk/model/ImportJSON.h"

#include "smtk/model/Storage.h"
#include "smtk/model/Entity.h"
#include "smtk/model/Tessellation.h"
#include "smtk/model/Arrangement.h"

#include "cJSON.h"

#include <string.h>

#if defined(_WIN32) && !defined(__CYGWIN__)
# define snprintf _snprintf
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
          arr.details.push_back(eger);
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
    tess.coords.clear();
    if (node->type == cJSON_Array)
      {
      int numEntries = cJSON_GetArraySize(node);
      tess.coords.reserve(numEntries);
      cJSON* entry;
      for (entry = node->child; entry; entry = entry->next)
        {
        double coord;
        if (cJSON_GetRealValue(entry, coord) == 0)
          {
          tess.coords.push_back(coord);
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
    tess.conn.clear();
    if (node->type == cJSON_Array)
      {
      cJSON* entry;
      for (entry = node->child; entry; entry = entry->next)
        {
        long eger;
        if (cJSON_GetIntegerValue(entry, eger) == 0)
          {
          tess.conn.push_back(eger);
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

int ImportJSON::intoModel(
  const char* json, StoragePtr model)
{
  return ImportJSON::intoModel(json, model.get());
}

int ImportJSON::intoModel(
  const char* json, Storage* model)
{
  int status = 0;
  if (!json || !json[0] || !model)
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
  if (mtyp && mtyp->type == cJSON_String && mtyp->valuestring && !strcmp(mtyp->valuestring,"Storage"))
    {
    cJSON* body = cJSON_GetObjectItem(root, "topo");
    status = ImportJSON::ofStorage(body, model);
    }

  cJSON_Delete(root);
  return status;
}

int ImportJSON::ofStorage(
  cJSON* dict, Storage* model)
{
  if (!dict || !model)
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
    status &= ImportJSON::ofStorageEntity(uid, curChild, model);
    status &= ImportJSON::ofStorageArrangement(uid, curChild, model);
    status &= ImportJSON::ofStorageTessellation(uid, curChild, model);
    status &= ImportJSON::ofStorageFloatProperties(uid, curChild, model);
    status &= ImportJSON::ofStorageStringProperties(uid, curChild, model);
    status &= ImportJSON::ofStorageIntegerProperties(uid, curChild, model);
    }
  return status;
}

int ImportJSON::ofStorageEntity(
  const UUID& uid, cJSON* cellRec, Storage* model)
{
  long dim;
  long entityFlags;
  int status = 0;
  status |= cJSON_GetObjectIntegerValue(cellRec, "d", dim);
  status |= cJSON_GetObjectIntegerValue(cellRec, "e", entityFlags);
  if (status == 0)
    {
    UUIDWithEntity iter = model->setEntityOfTypeAndDimension(uid, entityFlags, dim);
    // Ignore status from these as they need not be present:
    cJSON_GetObjectUUIDArray(cellRec, "r", iter->second.relations());
    }
  return status ? 0 : 1;
}

int ImportJSON::ofStorageArrangement(
  const UUID& uid, cJSON* dict, Storage* model)
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
      for (cJSON* arr = arrangements->child; arr; arr = arr->next)
        {
        if (arr->type == cJSON_Array)
          {
          Arrangement a;
          if (cJSON_GetArrangement(arr, a) > 0)
            {
            model->arrangeEntity(uid, k, a);
            }
          }
        }
      }
    }
  return 1;
}

int ImportJSON::ofStorageTessellation(
  const UUID& uid, cJSON* dict, Storage* model)
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
  UUIDsToTessellations::iterator tessIt = model->tessellations().find(uid);
  if (tessIt == model->tessellations().end())
    {
    Tessellation blank;
    tessIt = model->tessellations().insert(
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

int ImportJSON::ofStorageFloatProperties(const smtk::util::UUID& uid, cJSON* dict, Storage* model)
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
    model->setFloatProperty(uid, floatProp->string, propVal);
    }
  return status ? 0 : 1;
}

int ImportJSON::ofStorageStringProperties(const smtk::util::UUID& uid, cJSON* dict, Storage* model)
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
    model->setStringProperty(uid, stringProp->string, propVal);
    }
  return status ? 0 : 1;
}

int ImportJSON::ofStorageIntegerProperties(const smtk::util::UUID& uid, cJSON* dict, Storage* model)
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
    model->setIntegerProperty(uid, integerProp->string, propVal);
    }
  return status ? 0 : 1;
}

  }
}
