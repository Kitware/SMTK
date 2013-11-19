#include "smtk/model/ExportJSON.h"

#include "smtk/model/Storage.h"
#include "smtk/model/Entity.h"
#include "smtk/model/Tessellation.h"
#include "smtk/model/Arrangement.h"

#include "cJSON.h"

using namespace smtk::util;

// Some cJSON helpers
namespace {
  cJSON* cJSON_CreateUUIDArray(smtk::util::UUID* uids, unsigned count)
    {
    cJSON* a = cJSON_CreateArray();
    for (unsigned i = 0; i < count; ++i)
      {
      cJSON_AddItemToArray(a, cJSON_CreateString(uids[i].toString().c_str()));
      }
    return a;
    }
}

namespace smtk {
  namespace model {

using smtk::util::UUID;

cJSON* ExportJSON::fromUUIDs(const UUIDs& uids)
{
  cJSON* a = cJSON_CreateArray();
  for (UUIDs::const_iterator it = uids.begin(); it != uids.end(); ++it)
    {
    cJSON_AddItemToArray(a, cJSON_CreateString(it->toString().c_str()));
    }
  return a;
}

int ExportJSON::fromModel(cJSON* json, Storage* model)
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

  cJSON* mtyp = cJSON_CreateString("Storage");
  cJSON_AddItemToObject(json, "type", mtyp);
  status = ExportJSON::forStorage(body, model);

  return status;
}

int ExportJSON::forStorage(
  cJSON* dict, Storage* model)
{
  if (!dict || !model)
    {
    return 0;
    }
  int status = 1;
  UUIDWithEntity it;
  for (it = model->topology().begin(); it != model->topology().end(); ++it)
    {
    cJSON* curChild = cJSON_CreateObject();
      {
      std::string suid = it->first.toString();
      cJSON_AddItemToObject(dict, suid.c_str(), curChild);
      }
    status &= ExportJSON::forStorageEntity(it, curChild, model);
    status &= ExportJSON::forStorageArrangement(
      model->arrangements().find(it->first), curChild, model);
    status &= ExportJSON::forStorageTessellation(it->first, curChild, model);
    status &= ExportJSON::forStorageFloatProperties(it->first, curChild, model);
    status &= ExportJSON::forStorageStringProperties(it->first, curChild, model);
    status &= ExportJSON::forStorageIntegerProperties(it->first, curChild, model);
    }
  return status;
}

std::string ExportJSON::fromModel(StoragePtr model)
{
  cJSON* top = cJSON_CreateObject();
  ExportJSON::fromModel(top, model.get());
  std::string result(cJSON_Print(top));
  cJSON_Delete(top);
  return result;
}

int ExportJSON::forStorageEntity(
  UUIDWithEntity& entry, cJSON* cellRec, Storage* model)
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

int ExportJSON::forStorageArrangement(
  const UUIDWithArrangementDictionary& entry, cJSON* dict, Storage* model)
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
          cJSON_CreateIntArray(&(ait->details[0]), ait->details.size()));
        }
      }
    }
  return 1;
}

int ExportJSON::forStorageTessellation(
  const smtk::util::UUID& uid, cJSON* dict, Storage* model)
{
  UUIDWithTessellation tessIt = model->tessellations().find(uid);
  if (
    tessIt == model->tessellations().end() ||
    tessIt->second.coords.empty()
    )
    { // No tessellation? Not a problem.
    return 1;
    }
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
      &tessIt->second.coords[0],
      tessIt->second.coords.size()));
  cJSON_AddItemToObject(tess, "faces", cJSON_CreateIntArray(
      tessIt->second.conn.empty() ? NULL : &tessIt->second.conn[0],
      tessIt->second.conn.size()));
  cJSON_AddItemToObject(dict, "t", tess);
  return 1;
}

int ExportJSON::forStorageFloatProperties(const smtk::util::UUID& uid, cJSON*, Storage* model)
{
  int status = 0;
  return status ? 0 : 1;
}

int ExportJSON::forStorageStringProperties(const smtk::util::UUID& uid, cJSON*, Storage* model)
{
  int status = 0;
  return status ? 0 : 1;
}

int ExportJSON::forStorageIntegerProperties(const smtk::util::UUID& uid, cJSON*, Storage* model)
{
  int status = 0;
  return status ? 0 : 1;
}

  }
}
