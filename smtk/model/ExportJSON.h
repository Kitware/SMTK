#ifndef __smtk_model_ExportJSON_h
#define __smtk_model_ExportJSON_h

#include "smtk/SMTKCoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/PublicPointerDefs.h" // For ManagerPtr
#include "smtk/model/Manager.h" // For UUIDWithEntity
#include "smtk/util/SystemConfig.h"

#include "smtk/util/UUID.h"

struct cJSON;

namespace smtk {
  namespace model {

class Manager;

/**\brief Export an SMTK model into a JSON-formatted string.
  *
  * Methods are also provided for creating cJSON nodes representing
  * individual records and groups of records from SMTK storage (a model
  * manager).
  * These may be used to provide concise answers to specific queries
  * and avoid storing or transmitting a potentially-large string.
  */
class SMTKCORE_EXPORT ExportJSON
{
public:
  static cJSON* fromUUIDs(const smtk::util::UUIDs& uids);

  static int fromModel(cJSON* json, ManagerPtr model);
  static std::string fromModel(ManagerPtr model);

  static int forManager(cJSON* body, ManagerPtr model);
  static int forManagerEntity(UUIDWithEntity& entry, cJSON*, ManagerPtr model);
  static int forManagerArrangement(const UUIDWithArrangementDictionary& entry, cJSON*, ManagerPtr model);
  static int forManagerTessellation(const smtk::util::UUID& uid, cJSON*, ManagerPtr model);
  static int forManagerFloatProperties(const smtk::util::UUID& uid, cJSON*, ManagerPtr model);
  static int forManagerStringProperties(const smtk::util::UUID& uid, cJSON*, ManagerPtr model);
  static int forManagerIntegerProperties(const smtk::util::UUID& uid, cJSON*, ManagerPtr model);
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_ExportJSON_h
