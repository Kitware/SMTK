#ifndef smtk_ex_CounterOperation_h
#define smtk_ex_CounterOperation_h

#include "smtk/operation/XMLOperation.h"

#include <string>

// ++ 1 ++
namespace ex
{

class CounterOperation : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(ex::CounterOperation);
  smtkCreateMacro(CounterOperation);
  smtkSharedFromThisMacro(smtk::operation::XMLOperation);
  smtkSuperclassMacro(Operation);
  // ...
  // -- 1 --

  // ++ 2 ++
protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
  // -- 2 --
};

} // namespace ex

#endif // smtk_ex_CounterOperation_h
