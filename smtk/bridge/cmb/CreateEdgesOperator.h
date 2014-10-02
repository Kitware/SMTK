#ifndef __smtk_bridge_cmb_CreateEdgesOperator_h
#define __smtk_bridge_cmb_CreateEdgesOperator_h

#include "smtk/bridge/cmb/cmbBridgeExports.h"
#include "smtk/model/Operator.h"
#include "vtkCreateModelEdgesOperator.h"
#include "vtkNew.h"

namespace smtk {
  namespace bridge {
    namespace cmb {

class Bridge;

class SMTKCMBBRIDGE_EXPORT CreateEdgesOperator : public smtk::model::Operator
{
public:
  smtkTypeMacro(CreateEdgesOperator);
  smtkCreateMacro(CreateEdgesOperator);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

  virtual bool ableToOperate();

protected:
  CreateEdgesOperator();
  virtual smtk::model::OperatorResult operateInternal();
  Bridge* cmbBridge() const;

  vtkNew<vtkCreateModelEdgesOperator> m_op;
};

    } // namespace cmb
  } // namespace bridge
} // namespace smtk

#endif // __smtk_bridge_cmb_CreateEdgesOperator_h
