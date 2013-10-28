#ifndef __smtk_model_ExportJSON_h
#define __smtk_model_ExportJSON_h

#include "smtk/SMTKCoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/model/ModelBody.h"

#include "smtk/util/UUID.h"

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
  static int ForModelBodyLink(UUIDWithLink& entry, cJSON*, ModelBody* model);
  static int ForModelBodyArrangement(const UUIDWithArrangementDictionary& entry, cJSON*, ModelBody* model);
  static int ForModelBodyTessellation(const smtk::util::UUID& uid, cJSON*, ModelBody* model);
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_ExportJSON_h
