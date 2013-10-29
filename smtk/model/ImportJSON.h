#ifndef __smtk_model_ImportJSON_h
#define __smtk_model_ImportJSON_h

#include "smtk/SMTKCoreExports.h" // For SMTKCORE_EXPORT macro.

#include "smtk/util/UUID.h"

struct cJSON;

namespace smtk {
  namespace model {

class ModelBody;

class SMTKCORE_EXPORT ImportJSON
{
public:
  static int intoModel(const char* json, ModelBody* model);
  static int ofModelBody(cJSON* body, ModelBody* model);
  static int ofModelBodyLink(const smtk::util::UUID& uid, cJSON*, ModelBody* model);
  static int ofModelBodyArrangement(const smtk::util::UUID& uid, cJSON*, ModelBody* model);
  static int ofModelBodyTessellation(const smtk::util::UUID& uid, cJSON*, ModelBody* model);
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_ImportJSON_h
