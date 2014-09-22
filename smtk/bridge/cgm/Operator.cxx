#include "smtk/cgm/Operator.h"
#include "smtk/cgm/Bridge.h"

namespace cgmsmtk {
  namespace cgm {

/// Return a shared pointer to the bridge backing a CGM operator.
Bridge* Operator::cgmBridge()
{
  return dynamic_cast<cgmsmtk::cgm::Bridge*>(this->bridge());
}

  } // namespace cgm
} // namespace cgmsmtk
