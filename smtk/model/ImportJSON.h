#ifndef __smtk_model_ImportJSON_h
#define __smtk_model_ImportJSON_h

#include "smtk/SMTKCoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/PublicPointerDefs.h" // For StoragePtr

#include "smtk/util/UUID.h"

struct cJSON;

namespace smtk {
  namespace model {

class Storage;

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
  static int intoModel(const char* json, StoragePtr model);
  static int ofStorage(cJSON* body, StoragePtr model);
  static int ofStorageEntity(const smtk::util::UUID& uid, cJSON*, StoragePtr model);
  static int ofStorageArrangement(const smtk::util::UUID& uid, cJSON*, StoragePtr model);
  static int ofStorageTessellation(const smtk::util::UUID& uid, cJSON*, StoragePtr model);
  static int ofStorageFloatProperties(const smtk::util::UUID& uid, cJSON*, StoragePtr model);
  static int ofStorageStringProperties(const smtk::util::UUID& uid, cJSON*, StoragePtr model);
  static int ofStorageIntegerProperties(const smtk::util::UUID& uid, cJSON*, StoragePtr model);
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_ImportJSON_h
