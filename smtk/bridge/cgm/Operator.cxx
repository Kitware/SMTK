#include "smtk/bridge/cgm/Operator.h"
#include "smtk/bridge/cgm/Bridge.h"

namespace smtk {
  namespace bridge {
    namespace cgm {

/// Return a shared pointer to the bridge backing a CGM operator.
Bridge* Operator::cgmBridge()
{
  return dynamic_cast<smtk::bridge::cgmBridge*>(this->bridge());
}

} // namespace cgm
  } //namespace bridge
} // namespace smtk
