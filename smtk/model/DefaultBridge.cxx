#include "smtk/model/DefaultBridge.h"

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
