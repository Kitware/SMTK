#ifndef __cmbsmtk_cmb_ReadOperator_h
#define __cmbsmtk_cmb_ReadOperator_h

#include "smtk/model/Operator.h"
#include "vtkCMBModelReadOperator.h"
#include "vtkNew.h"

namespace cmbsmtk {
  namespace cmb {

class Bridge;

/**\brief Read a CMB discrete model file.
  *
  * This requires the file to be of type/extension "cmb" (which
  * is really just a VTK XML polydata file).
  */
class VTKCMBDISCRETEMODEL_EXPORT ReadOperator : public smtk::model::Operator
{
public:
  smtkTypeMacro(ReadOperator);
  smtkCreateMacro(ReadOperator);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

  virtual bool ableToOperate();

protected:
  ReadOperator();
  virtual smtk::model::OperatorResult operateInternal();
  Bridge* cmbBridge() const;

  vtkNew<vtkCMBModelReadOperator> m_op;
};

  } // namespace cmb
} // namespace cmbsmtk

#endif // __cmbsmtk_cmb_ReadOperator_h
