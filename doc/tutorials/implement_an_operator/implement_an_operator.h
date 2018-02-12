#ifndef __smtk_ex_CounterOperation_h
#define __smtk_ex_CounterOperation_h

#include "smtk/model/Operation.h"

#include <string>

// ++ 1 ++
namespace ex
{

class CounterOperation : public smtk::model::Operator
{
public:
  smtkTypeMacro(CounterOperation);
  smtkCreateMacro(CounterOperation);
  smtkSharedFromThisMacro(Operation);
  smtkDeclareModelOperation();
  // ...
  // -- 1 --

  // ++ 2 ++
  bool ableToOperate() override { return this->ensureSpecification(); }

protected:
  smtk::model::OperationResult operateInternal() override;
  // -- 2 --
};

} // namespace ex

#endif // __smtk_ex_CounterOperation_h
