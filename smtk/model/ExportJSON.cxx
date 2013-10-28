#include "smtk/model/ExportJSON.h"

#include "smtk/model/ModelBody.h"
#include "smtk/model/Link.h"
#include "smtk/model/Tessellation.h"
#include "smtk/model/Arrangement.h"

#include "cJSON.h"

// Some cJSON helpers
namespace {
  cJSON* cJSON_CreateUUIDArray(smtk::util::UUID* uids, unsigned count)
    {
    cJSON* a = cJSON_CreateArray();
    for (unsigned i = 0; i < count; ++i)
      {
      cJSON_AddItemToArray(a, cJSON_CreateString(uids[i].ToString().c_str()));
      }
    return a;
    }
}

namespace smtk {
  namespace model {

using smtk::util::UUID;

cJSON* ExportJSON::FromUUIDs(const UUIDs& uids)
{
  cJSON* a = cJSON_CreateArray();
  for (UUIDs::iterator it = uids.begin(); it != uids.end(); ++it)
    {
    cJSON_AddItemToArray(a, cJSON_CreateString(it->ToString().c_str()));
    }
  return a;
}

int ExportJSON::FromModel(cJSON* json, ModelBody* model)
{
  int status = 0;
  if (!json || !model)
    {
    std::cerr << "Invalid arguments.\n";
    return status;
    }

  cJSON* body = cJSON_CreateObject();
  switch(json->type)
    {
  case cJSON_Object:
    cJSON_AddItemToObject(json, "topo", body);
    break;
  case cJSON_Array:
    cJSON_AddItemToArray(json, body);
    break;
  case cJSON_NULL:
  case cJSON_Number:
  case cJSON_String:
  default:
    std::cerr << "Invalid toplevel JSON type (" << json->type << ").\n";
    return status;
    break;
    }

  cJSON* mtyp = cJSON_CreateString("ModelBody");
  cJSON_AddItemToObject(json, "type", mtyp);
  status = ExportJSON::ForModelBody(body, model);

  return status;
}

int ExportJSON::ForModelBody(
  cJSON* dict, ModelBody* model)
{
  if (!dict || !model)
    {
    return 0;
    }
  int status = 1;
  UUIDWithLink it;
  for (it = model->topology().begin(); it != model->topology().end(); ++it)
    {
    cJSON* curChild = cJSON_CreateObject();
      {
      std::string suid = it->first.ToString();
      cJSON_AddItemToObject(dict, suid.c_str(), curChild);
      }
    status &= ExportJSON::ForModelBodyLink(it, curChild, model);
    status &= ExportJSON::ForModelBodyArrangement(
      model->arrangements().find(it->first), curChild, model);
    status &= ExportJSON::ForModelBodyTessellation(it->first, curChild, model);
    }
  return status;
}

int ExportJSON::ForModelBodyLink(
  UUIDWithLink& entry, cJSON* cellRec, ModelBody* model)
{
  (void)model;
  cJSON* ent = cJSON_CreateNumber(entry->second.entityFlags());
  cJSON* dim = cJSON_CreateNumber(entry->second.dimension());
  cJSON_AddItemToObject(cellRec, "e", ent);
  cJSON_AddItemToObject(cellRec, "d", dim);
  cJSON_AddItemToObject(cellRec, "r",
    cJSON_CreateUUIDArray(
      &entry->second.relations()[0],
      entry->second.relations().size()));
  return 1;
}

int ExportJSON::ForModelBodyArrangement(
  const UUIDWithArrangementDictionary& entry, cJSON* dict, ModelBody* model)
{
  if (entry == model->arrangements().end())
    {
    return 0;
    }
  ArrangementKindWithArrangements it;
  cJSON* arrNode = cJSON_CreateObject();
  cJSON_AddItemToObject(dict, "a", arrNode);
  for (it = entry->second.begin(); it != entry->second.end(); ++it)
    {
    Arrangements& arr(it->second);
    if (!arr.empty())
      {
      cJSON* kindNode = cJSON_CreateArray();
      cJSON_AddItemToObject(arrNode, smtk::model::AbbreviationForArrangementKind(it->first).c_str(), kindNode);
      Arrangements::iterator ait;
      for (ait = arr.begin(); ait != arr.end(); ++ait)
        {
        cJSON_AddItemToArray(kindNode,
          cJSON_CreateIntArray(&(ait->Details[0]), ait->Details.size()));
        }
      }
    }
  return 1;
}

int ExportJSON::ForModelBodyTessellation(
  const UUID& uid, cJSON* dict, ModelBody* model)
{
  (void)uid;
  (void)dict;
  (void)model;
  //  "metadata": { "formatVersion" : 3 },
  //  "vertices": [ 0,0,0, 0,0,1, 1,0,1, 1,0,0, ... ],
  //  "normals":  [ 0,1,0, ... ],
  //  "faces": [
  //    0, 0,1,2, // tri
  //    1, 0,1,2,3, // quad
  //    32, 0,1,2, // tri w/ per-vert norm
  cJSON* tess = cJSON_CreateObject();
  //cJSON* meta = cJSON_CreateObject();
  cJSON* fmt = cJSON_CreateObject();
  cJSON_AddItemToObject(fmt,"formatVersion", cJSON_CreateNumber(3));
  //cJSON_AddItemToObject(meta, "metadata", fmt);
  //cJSON_AddItemToObject(tess, "3js", meta);
  cJSON_AddItemToObject(tess, "metadata", fmt);
  cJSON_AddItemToObject(tess, "vertices", cJSON_CreateDoubleArray(
      &model->tessellations()[uid].Coords[0],
      model->tessellations()[uid].Coords.size()));
  cJSON_AddItemToObject(tess, "faces", cJSON_CreateIntArray(
      &model->tessellations()[uid].Conn[0],
      model->tessellations()[uid].Conn.size()));
  cJSON_AddItemToObject(dict, "t", tess);
  return 1;
}

  }
}
