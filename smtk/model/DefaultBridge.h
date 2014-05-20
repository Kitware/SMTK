#ifndef __smtk_model_DefaultBridge_h
#define __smtk_model_DefaultBridge_h

#include "smtk/model/Bridge.h"

namespace smtk {
  namespace model {

/**\brief A bridge that does no transcription.
  *
  * In other words, this bridge marks models as being "native" to SMTK.
  */
class DefaultBridge : public Bridge
{
public:
  smtkTypeMacro(DefaultBridge);
  smtkCreateMacro(Bridge);
  smtkSharedFromThisMacro(Bridge);
  smtkDeclareModelingKernel();

protected:
  virtual BridgedInfoBits transcribeInternal(const Cursor& entity, BridgedInfoBits flags);
};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_DefaultBridge_h
