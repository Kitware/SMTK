#ifndef __smtk_model_ImportJSON_h
#define __smtk_model_ImportJSON_h

#include "smtk/SMTKCoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/PublicPointerDefs.h" // For StoragePtr

#include "smtk/util/UUID.h"

struct cJSON;

namespace smtk {
  namespace model {

class Storage;

class SMTKCORE_EXPORT ImportJSON
{
public:
  static int intoModel(const char* json, StoragePtr model);
  static int intoModel(const char* json, Storage* model);
  static int ofStorage(cJSON* body, Storage* model);
  static int ofStorageEntity(const smtk::util::UUID& uid, cJSON*, Storage* model);
  static int ofStorageArrangement(const smtk::util::UUID& uid, cJSON*, Storage* model);
  static int ofStorageTessellation(const smtk::util::UUID& uid, cJSON*, Storage* model);
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_ImportJSON_h
