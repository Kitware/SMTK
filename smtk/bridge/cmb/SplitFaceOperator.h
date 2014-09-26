#ifndef __cmbsmtk_cmb_SplitFaceOperator_h
#define __cmbsmtk_cmb_SplitFaceOperator_h

#include "smtk/model/Operator.h"
#include "vtkSplitOperator.h"
#include "vtkNew.h"

namespace cmbsmtk {
  namespace cmb {

class Bridge;

class VTKCMBDISCRETEMODEL_EXPORT SplitFaceOperator : public smtk::model::Operator
{
public:
  smtkTypeMacro(SplitFaceOperator);
  smtkCreateMacro(SplitFaceOperator);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

  virtual bool ableToOperate();

protected:
  SplitFaceOperator();
  virtual smtk::model::OperatorResult operateInternal();
  Bridge* cmbBridge() const;
  int fetchCMBFaceId() const;

  vtkNew<vtkSplitOperator> m_op;
};

  } // namespace cmb
} // namespace cmbsmtk

#endif // __cmbsmtk_cmb_SplitFaceOperator_h
