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

/**\brief Indicate what data should be exported to JSON.
  *
  */
enum JSONFlags
{
  JSON_NOTHING       = 0x00, //!< Export nothing.
  JSON_ENTITIES      = 0x01, //!< Export model-entity entries in Manager (not including tessellations or properties).
  JSON_BRIDGES       = 0x02, //!< Export bridge sessions (i.e., bridge session IDs, the bridge type, and operators).
  JSON_TESSELLATIONS = 0x04, //!< Export tessellations of model-entity entries in the Manager.
  JSON_PROPERTIES    = 0x08, //!< Export string/float/integer properties of model-entity entries in the Manager.
  JSON_DEFAULT       = 0xff  //!< By default, export everything.
};

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

  static int fromModel(cJSON* json, ManagerPtr modelMgr, JSONFlags sections = JSON_DEFAULT);
  static std::string fromModel(ManagerPtr modelMgr, JSONFlags sections = JSON_DEFAULT);

  static int forManager(cJSON* body, cJSON* sess, ManagerPtr modelMgr, JSONFlags sections = JSON_DEFAULT);
  static int forManagerEntity(UUIDWithEntity& entry, cJSON*, ManagerPtr modelMgr);
  static int forManagerArrangement(const UUIDWithArrangementDictionary& entry, cJSON*, ManagerPtr modelMgr);
  static int forManagerTessellation(const smtk::util::UUID& uid, cJSON*, ManagerPtr modelMgr);
  static int forManagerFloatProperties(const smtk::util::UUID& uid, cJSON*, ManagerPtr modelMgr);
  static int forManagerStringProperties(const smtk::util::UUID& uid, cJSON*, ManagerPtr modelMgr);
  static int forManagerIntegerProperties(const smtk::util::UUID& uid, cJSON*, ManagerPtr modelMgr);
  static int forManagerBridgeSession(const smtk::util::UUID& uid, cJSON*, ManagerPtr modelMgr);
  static int forModelOperators(const smtk::util::UUID& uid, cJSON*, ManagerPtr modelMgr);
  static int forOperators(Operators& ops, cJSON*);
  static int forOperator(OperatorPtr op, cJSON*);

  // Low-level helpers:
  static cJSON* createStringArray(std::vector<std::string>& arr);
  static cJSON* createUUIDArray(std::vector<smtk::util::UUID>& arr);
  static cJSON* createIntegerArray(std::vector<long>& arr);
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_ExportJSON_h
