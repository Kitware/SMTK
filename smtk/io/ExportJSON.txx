#ifndef __smtk_io_ExportJSON_txx
#define __smtk_io_ExportJSON_txx

#include "smtk/io/ExportJSON.h"

#include "smtk/model/EntityIterator.h"

#include "cJSON.h"

namespace smtk {
  namespace io {

/**\brief Populate the \a json node with the record(s) related to given \a entities.
  *
  */
template<typename T>
int ExportJSON::forEntities(
  cJSON* json,
  const T& entities,
  smtk::model::IteratorStyle relatedEntities,
  JSONFlags sections)
{
  using namespace smtk::model;

  if (!json || sections == JSON_NOTHING)
    return 1;

  EntityIterator iter;
  iter.traverse(entities.begin(), entities.end(), relatedEntities);
  int status = 1;
  for (iter.begin(); !iter.isAtEnd(); ++iter)
    {
    // Pull the first entry off the queue.
    EntityRef ent = *iter;

    // Generate JSON for the queued entity
    ManagerPtr modelMgr = ent.manager();
    UUIDWithEntity it = modelMgr->topology().find(ent.entity());
    if (
      (it == ent.manager()->topology().end()) ||
      ((it->second.entityFlags() & SESSION) && !(sections & JSON_SESSIONS)))
      continue;

    cJSON* curChild = cJSON_CreateObject();
      {
      std::string suid = it->first.toString();
      cJSON_AddItemToObject(json, suid.c_str(), curChild);
      }
    if (sections & JSON_ENTITIES)
      {
      status &= ExportJSON::forManagerEntity(it, curChild, modelMgr);
      status &= ExportJSON::forManagerArrangement(
        modelMgr->arrangements().find(it->first), curChild, modelMgr);
      }
    if (sections & JSON_TESSELLATIONS)
      status &= ExportJSON::forManagerTessellation(it->first, curChild, modelMgr);
    if (sections & JSON_ANALYSISMESH)
      status &= ExportJSON::forManagerAnalysis(it->first, curChild, modelMgr);
    if (sections & JSON_PROPERTIES)
      {
      status &= ExportJSON::forManagerFloatProperties(it->first, curChild, modelMgr);
      status &= ExportJSON::forManagerStringProperties(it->first, curChild, modelMgr);
      status &= ExportJSON::forManagerIntegerProperties(it->first, curChild, modelMgr);
      }
    }
  return 0;
}

/**\brief Populate the \a json node with the record(s) related to given \a entities.
  *
  */
template<typename T>
std::string ExportJSON::forEntities(
  const T& entities,
  smtk::model::IteratorStyle relatedEntities,
  JSONFlags sections)
{
  using namespace smtk::model;

  cJSON* top = cJSON_CreateObject();
  ExportJSON::forEntities(top, entities, relatedEntities, sections);
  char* json = cJSON_Print(top);
  std::string result(json);
  free(json);
  cJSON_Delete(top);
  return result;
}

  } // namespace io
} // namespace smtk

#endif // __smtk_io_ExportJSON_txx
