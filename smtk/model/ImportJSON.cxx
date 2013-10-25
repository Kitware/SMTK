#include "smtk/model/ImportJSON.h"

#include "smtk/model/ModelBody.h"
#include "smtk/model/Cell.h"
#include "smtk/model/Tessellation.h"
#include "smtk/model/Arrangement.h"

#include "cJSON.h"

#include <string.h>

// Some cJSON helpers
namespace {
  int cJSON_GetIntegerValue(cJSON* valItem, int& val)
    {
    switch (valItem->type)
      {
    case cJSON_Number:
      val = valItem->valueint;
      return 1;
    case cJSON_String:
      if (valItem->valuestring)
        {
        char* strEnd;
        long tmp = strtol(valItem->valuestring, &strEnd, 10);
        // Only accept the conversion if the entire string is consumed:
        if (valItem->valuestring[0] && !*strEnd)
          {
          val = static_cast<int>(tmp);
          return 1;
          }
        }
    default:
      break;
      }
    return 0;
    }
  int cJSON_GetObjectIntegerValue(cJSON* node, const char* name, int& val)
    {
    cJSON* valItem = cJSON_GetObjectItem(node, name);
    if (valItem)
      {
      return cJSON_GetIntegerValue(valItem, val);
      }
    return 0;
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
        return 0;
        }
      }
    return 1;
    }
  int cJSON_GetObjectUUIDArray(cJSON* node, const char* name, std::vector<smtk::util::UUID>& uids)
    {
    cJSON* valItem = cJSON_GetObjectItem(node, name);
    if (valItem && valItem->type == cJSON_Array)
      {
      return cJSON_GetUUIDArray(valItem->child, uids);
      }
    return 0;
    }
  int cJSON_GetArrangement(cJSON* node, smtk::model::Arrangement& arr)
    {
    int count = 0;
    if (node->type == cJSON_Array)
      {
      cJSON* entry;
      for (entry = node->child; entry; entry = entry->next)
        {
        int eger;
        if (cJSON_GetIntegerValue(entry, eger))
          {
          arr.Details.push_back(eger);
          ++count;
          }
        }
      }
    return count;
    }
}

namespace smtk {
  namespace model {

using smtk::util::UUID;

int ImportJSON::IntoModel(
  const char* json, ModelBody* model)
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
  if (mtyp && mtyp->type == cJSON_String && mtyp->valuestring && !strcmp(mtyp->valuestring,"ModelBody"))
    {
    cJSON* body = cJSON_GetObjectItem(root, "topo");
    status = ImportJSON::OfModelBody(body, model);
    }

  cJSON_Delete(root);
  return status;
}

int ImportJSON::OfModelBody(
  cJSON* dict, ModelBody* model)
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
    if (uid.IsNull())
      {
      std::cerr << "Skipping malformed UUID: " << curChild->string << "\n";
      continue;
      }
    status &= ImportJSON::OfModelBodyCell(uid, curChild, model);
    status &= ImportJSON::OfModelBodyArrangement(uid, curChild, model);
    status &= ImportJSON::OfModelBodyTessellation(uid, curChild, model);
    }
  return status;
}

int ImportJSON::OfModelBodyCell(
  const UUID& uid, cJSON* cellRec, ModelBody* model)
{
  int dim;
  int status = cJSON_GetObjectIntegerValue(cellRec, "d", dim);
  if (status)
    {
    UUIDWithCell iter = model->SetCellOfDimension(uid, dim);
    // Ignore status from these as they need not be present:
    cJSON_GetObjectUUIDArray(cellRec, "r", iter->second.relations());
    }
  return status;
}

int ImportJSON::OfModelBodyArrangement(
  const UUID& uid, cJSON* dict, ModelBody* model)
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
            model->ArrangeCell(uid, k, a);
            }
          }
        }
      }
    }
  return 1;
}

int ImportJSON::OfModelBodyTessellation(
  const UUID& uid, cJSON* dict, ModelBody* model)
{
  (void)uid;
  (void)dict;
  (void)model;
  return 1;
}

  }
}
