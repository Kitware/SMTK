#ifndef __smtk_model_OperatorResult_h
#define __smtk_model_OperatorResult_h

#include "smtk/model/Parameter.h"

#include "smtk/util/SharedFromThis.h"
#include "smtk/util/UUID.h"

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

  Parameters parameters() const;
  const Parameter& parameter(const std::string& name) const;
  Parameter parameter(const std::string& name);
  virtual void setParameters(const Parameters& p);
  virtual void setParameter(const Parameter& p);
  bool hasFloatParameter(const std::string& name, int minSize = 1, int maxSize = -1, bool validate = true) const;
  bool hasStringParameter(const std::string& name, int minSize = 1, int maxSize = -1, bool validate = true) const;
  bool hasIntegerParameter(const std::string& name, int minSize = 1, int maxSize = -1, bool validate = true) const;
  bool hasUUIDParameter(const std::string& name, int minSize = 1, int maxSize = -1, bool validate = true) const;

protected:
  OperatorOutcome m_outcome;
  Parameters m_parameters;

  bool checkParameterSize(int psize, int minSize, int maxSize) const;
};

  } // model namespace
} // smtk namespace

#endif // __smtk_model_OperatorResult_h
