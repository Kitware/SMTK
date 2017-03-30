#ifndef __smtk_ex_CounterOperator_h
#define __smtk_ex_CounterOperator_h

#include "smtk/model/Operator.h"

#include <string>

// ++ 1 ++
namespace ex
{

class CounterOperator : public smtk::model::Operator
{
public:
  smtkTypeMacro(CounterOperator);
  smtkCreateMacro(CounterOperator);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();
  // ...
  // -- 1 --

  // ++ 2 ++
  virtual bool ableToOperate() { return this->ensureSpecification(); }

protected:
  virtual smtk::model::OperatorResult operateInternal();
  // -- 2 --
};

} // namespace ex

#endif // __smtk_ex_CounterOperator_h
