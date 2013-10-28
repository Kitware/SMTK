#ifndef __smtk_model_ImportJSON_h
#define __smtk_model_ImportJSON_h

#include "smtk/SMTKCoreExports.h" // For SMTKCORE_EXPORT macro.

#include "smtk/util/UUID.h"

struct cJSON;

namespace smtk {
  namespace model {

using smtk::util::UUID;
class ModelBody;

class SMTKCORE_EXPORT ImportJSON
{
public:
  static int IntoModel(const char* json, ModelBody* model);
  static int OfModelBody(cJSON* body, ModelBody* model);
  static int OfModelBodyLink(const UUID& uid, cJSON*, ModelBody* model);
  static int OfModelBodyArrangement(const UUID& uid, cJSON*, ModelBody* model);
  static int OfModelBodyTessellation(const UUID& uid, cJSON*, ModelBody* model);
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_ImportJSON_h
