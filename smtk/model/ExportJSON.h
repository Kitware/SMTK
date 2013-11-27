#ifndef __smtk_model_ExportJSON_h
#define __smtk_model_ExportJSON_h

#include "smtk/SMTKCoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/PublicPointerDefs.h" // For StoragePtr
#include "smtk/model/Storage.h" // For UUIDWithEntity
#include "smtk/util/SystemConfig.h"

#include "smtk/util/UUID.h"

struct cJSON;

namespace smtk {
  namespace model {

class Storage;

class SMTKCORE_EXPORT ExportJSON
{
public:
  static cJSON* fromUUIDs(const smtk::util::UUIDs& uids);

  static int fromModel(cJSON* json, Storage* model);
  static std::string fromModel(StoragePtr model);

  static int forStorage(cJSON* body, Storage* model);
  static int forStorageEntity(UUIDWithEntity& entry, cJSON*, Storage* model);
  static int forStorageArrangement(const UUIDWithArrangementDictionary& entry, cJSON*, Storage* model);
  static int forStorageTessellation(const smtk::util::UUID& uid, cJSON*, Storage* model);
  static int forStorageFloatProperties(const smtk::util::UUID& uid, cJSON*, Storage* model);
  static int forStorageStringProperties(const smtk::util::UUID& uid, cJSON*, Storage* model);
  static int forStorageIntegerProperties(const smtk::util::UUID& uid, cJSON*, Storage* model);
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_ExportJSON_h
