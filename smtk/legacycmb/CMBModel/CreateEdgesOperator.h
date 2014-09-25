#ifndef __cmbsmtk_cmb_CreateEdgesOperator_h
#define __cmbsmtk_cmb_CreateEdgesOperator_h

#include "smtk/model/Operator.h"
#include "vtkCreateModelEdgesOperator.h"
#include "vtkNew.h"

namespace cmbsmtk {
  namespace cmb {

class Bridge;

class VTKCMBDISCRETEMODEL_EXPORT CreateEdgesOperator : public smtk::model::Operator
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
} // namespace cmbsmtk

#endif // __cmbsmtk_cmb_CreateEdgesOperator_h
