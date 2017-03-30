#ifndef __smtk_io_SaveJSON_txx
#define __smtk_io_SaveJSON_txx

#include "smtk/io/SaveJSON.h"

#include "smtk/model/EntityIterator.h"

#include "cJSON.h"

namespace smtk
{
namespace io
{

/**\brief Populate the \a json node with the record(s) related to given \a entities.
  *
  */
template <typename T>
int SaveJSON::forEntities(
  cJSON* json, const T& entities, smtk::model::IteratorStyle relatedEntities, JSONFlags sections)
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
    if ((it == ent.manager()->topology().end()) ||
      ((it->second.entityFlags() & SESSION) && !(sections & JSON_SESSIONS)))
      continue;

    cJSON* curChild = cJSON_CreateObject();
    {
      std::string suid = it->first.toString();
      cJSON_AddItemToObject(json, suid.c_str(), curChild);
    }
    if (sections & JSON_ENTITIES)
    {
      status &= SaveJSON::forManagerEntity(it, curChild, modelMgr);
      status &= SaveJSON::forManagerArrangement(
        modelMgr->arrangements().find(it->first), curChild, modelMgr);
    }
    if (sections & JSON_TESSELLATIONS)
      status &= SaveJSON::forManagerTessellation(it->first, curChild, modelMgr);
    if (sections & JSON_PROPERTIES)
    {
      status &= SaveJSON::forManagerFloatProperties(it->first, curChild, modelMgr);
      status &= SaveJSON::forManagerStringProperties(it->first, curChild, modelMgr);
      status &= SaveJSON::forManagerIntegerProperties(it->first, curChild, modelMgr);
    }
  }
  return status;
}

/**\brief Populate the \a json node with the record(s) related to given \a entities.
  *
  */
template <typename T>
std::string SaveJSON::forEntities(
  const T& entities, smtk::model::IteratorStyle relatedEntities, JSONFlags sections)
{
  using namespace smtk::model;

  cJSON* top = cJSON_CreateObject();
  SaveJSON::forEntities(top, entities, relatedEntities, sections);
  char* json = cJSON_Print(top);
  std::string result(json);
  free(json);
  cJSON_Delete(top);
  return result;
}

} // namespace io
} // namespace smtk

#endif // __smtk_io_SaveJSON_txx
