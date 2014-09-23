#ifndef __smtk_bridge_cgm_Operator_h
#define __smtk_bridge_cgm_Operator_h

#include "smtk/bridge/cgm/cgmSMTKExports.h"
#include "smtk/model/Operator.h"

namespace smtk {
  namespace bridge {
    namespace cgm {

class Bridge;

/**\brief An operator using the CGM kernel.
  *
  * This is a base class for actual CGM operators.
  * It provides convenience methods for accessing CGM-specific data
  * for its subclasses to use internally.
  */
class CGMSMTK_EXPORT Operator : public smtk::model::Operator
{
protected:
  Bridge* cgmBridge();
};

} // namespace cgm
  } //namespace bridge
} // namespace smtk

#endif // __smtk_bridge_cgm_Operator_h
