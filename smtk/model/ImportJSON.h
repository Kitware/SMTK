#ifndef __smtk_model_ImportJSON_h
#define __smtk_model_ImportJSON_h

#include "smtk/SMTKCoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/PublicPointerDefs.h" // For ManagerPtr

#include "smtk/util/UUID.h"

struct cJSON;

namespace smtk {
  namespace model {

class Manager;

/**\brief Import an SMTK model from JSON data.
  *
  * Methods are also provided for importing individual records
  * and groups of records directly from cJSON nodes.
  * These may be used to update relevant entities without
  * storing or transmitting a potentially-large string.
  */
class SMTKCORE_EXPORT ImportJSON
{
public:
  static int intoModel(const char* json, ManagerPtr manager);
  static int ofManager(cJSON* body, ManagerPtr manager);
  static int ofManagerEntity(const smtk::util::UUID& uid, cJSON*, ManagerPtr manager);
  static int ofManagerArrangement(const smtk::util::UUID& uid, cJSON*, ManagerPtr manager);
  static int ofManagerTessellation(const smtk::util::UUID& uid, cJSON*, ManagerPtr manager);
  static int ofManagerFloatProperties(const smtk::util::UUID& uid, cJSON*, ManagerPtr manager);
  static int ofManagerStringProperties(const smtk::util::UUID& uid, cJSON*, ManagerPtr manager);
  static int ofManagerIntegerProperties(const smtk::util::UUID& uid, cJSON*, ManagerPtr manager);
  static int ofRemoteBridgeSession(cJSON*, DefaultBridgePtr destBridge, ManagerPtr context);
  static int ofOperator(cJSON* node, OperatorPtr& op, ManagerPtr context);
  static int ofOperatorResult(cJSON* node, OperatorResult& resOut, smtk::attribute::Manager* opMgr);
  static int ofDanglingEntities(cJSON* node, ManagerPtr context);

  // Low-level helpers:
  static int getUUIDArrayFromJSON(cJSON* uidRec, std::vector<smtk::util::UUID>& uids);
  static int getStringArrayFromJSON(cJSON* arrayNode, std::vector<std::string>& text);
  static int getIntegerArrayFromJSON(cJSON* arrayNode, std::vector<long>& values);
  static int getRealArrayFromJSON(cJSON* arrayNode, std::vector<double>& values);
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_ImportJSON_h
