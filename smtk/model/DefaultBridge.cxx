#include "smtk/model/DefaultBridge.h"

#include "smtk/util/AutoInit.h"
#include "smtk/model/BRepModel.h"

namespace smtk {
  namespace model {

/// Indicate that, since we have no "backing store" model, the entire model is already present.
BridgedInfoBits DefaultBridge::transcribeInternal(const Cursor& entity, BridgedInfoBits flags)
{
  (void)entity;
  (void)flags;
  return BRIDGE_EVERYTHING;
}

  } // namespace model
} // namespace smtk

static const char* DefaultBridgeFileTypes[] = {
  ".json",
  NULL
};
smtkImplementsModelingKernel(native,DefaultBridgeFileTypes,smtk::model::DefaultBridge);
