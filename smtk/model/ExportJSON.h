#ifndef __smtk_model_ExportJSON_h
#define __smtk_model_ExportJSON_h

#include "smtk/SMTKCoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/model/ModelBody.h"

#include "smtk/model/UUID.h"

struct cJSON;

namespace smtk {
  namespace model {

class ModelBody;

class SMTKCORE_EXPORT ExportJSON
{
public:
  static cJSON* FromUUIDs(const UUIDs& uids);
  static int FromModel(cJSON* json, ModelBody* model);
  static int ForModelBody(cJSON* body, ModelBody* model);
  static int ForModelBodyCell(UUIDWithCell& entry, cJSON*, ModelBody* model);
  static int ForModelBodyArrangement(const UUIDWithArrangementDictionary& entry, cJSON*, ModelBody* model);
  static int ForModelBodyTessellation(const UUID& uid, cJSON*, ModelBody* model);
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_ExportJSON_h
