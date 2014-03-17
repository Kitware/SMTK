#ifndef __smtk_model_OperatorResult_h
#define __smtk_model_OperatorResult_h

#include "smtk/util/SharedFromThis.h"
//#include "smtk/SharedPtr.h"

#include "smtk/SMTKCoreExports.h" // For SMTKCORE_EXPORT macro.

namespace smtk {
  namespace model {

/// An enumeration of operation outcomes (or lacks thereof).
enum OperatorOutcome
{
  UNABLE_TO_OPERATE,   //!< The operator was misconfigured.
  OPERATION_CANCELED,  //!< An observer requested the operation be canceled. And it was.
  OPERATION_FAILED,    //!< The operator attempted to execute but encountered a problem.
  OPERATION_SUCCEEDED  //!< The operator succeeded.
};

/**\brief A class for reporting results of operations on a solid model.
  *
  * You may ask for the outcome of the operation (as an enumerated value),
  * and for a list of affected entities (not currently implemented).
  */
class SMTKCORE_EXPORT OperatorResult
{
public:
  smtkTypeMacro(OperatorResult);

  OperatorResult();
  OperatorResult(OperatorOutcome oc);

  OperatorOutcome outcome() const;

protected:
  OperatorOutcome m_outcome;
};

  } // model namespace
} // smtk namespace

#endif // __smtk_model_OperatorResult_h
