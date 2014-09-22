#ifndef __smtk_cgm_ReadOperator_h
#define __smtk_cgm_ReadOperator_h

#include "smtk/cgm/Operator.h"

namespace cgmsmtk {
  namespace cgm {

class CGMSMTK_EXPORT ReadOperator : public Operator
{
public:
  smtkTypeMacro(ReadOperator);
  smtkCreateMacro(ReadOperator);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

  virtual bool ableToOperate();

protected:
  virtual smtk::model::OperatorResult operateInternal();
};

  } // namespace cgm
} // namespace cgmsmtk

#endif // __smtk_cgm_ReadOperator_h
