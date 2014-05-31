#include "smtk/model/OperatorResult.h"

namespace smtk {
  namespace model {

OperatorResult::OperatorResult()
  : m_outcome(UNABLE_TO_OPERATE)
{
}

OperatorResult::OperatorResult(OperatorOutcome oc)
  : m_outcome(oc)
{
}

OperatorOutcome OperatorResult::outcome() const
{
  return this->m_outcome;
}

/// Return a copy of this result's parameters.
Parameters OperatorResult::parameters() const
{
  return this->m_parameters;
}

/// Return the parameter of the given name (or an invalid, uninitialized parameter if it does not exist)
const Parameter& OperatorResult::parameter(const std::string& pname) const
{
  static const Parameter invalid;
  Parameters::const_iterator it;
  if ((it = this->m_parameters.find(Parameter(pname))) != this->m_parameters.end())
    return *it;
  return invalid;
}

/// Return the parameter of the given name (or an invalid, uninitialized parameter if it does not exist)
Parameter OperatorResult::parameter(const std::string& pname)
{
  Parameters::const_iterator it;
  if ((it = this->m_parameters.find(Parameter(pname))) != this->m_parameters.end())
    return *it;
  return Parameter();
}

/**\brief Set all of the parameters to those given.
  *
  * This clears the entire set of current parameters and
  * then adds each parameter from the set \a p.
  */
void OperatorResult::setParameters(const Parameters& p)
{
  this->m_parameters.clear();
  Parameters::const_iterator it;
  for (it = p.begin(); it != p.end(); ++it)
    {
    this->setParameter(*it);
    }
}

/// Set the value of a parameter to \a p.
void OperatorResult::setParameter(const Parameter& p)
{
  Parameters::const_iterator old = this->m_parameters.find(p);
  if (old != this->m_parameters.end())
    {
    this->m_parameters.erase(*old);
    }
  this->m_parameters.insert(p);
}

/// Check whether a parameter of the given name exists and has an acceptable number of entries.
bool OperatorResult::hasFloatParameter(const std::string& pname, int minSize, int maxSize, bool validate) const
{
  bool ok = false;
  Parameters::const_iterator it;
  if ((it = this->m_parameters.find(Parameter(pname))) != this->m_parameters.end())
    {
    int psize = static_cast<int>(it->floatValues().size());
    ok = this->checkParameterSize(psize, minSize, maxSize);
    if (validate)
      const_cast<Parameter&>(*it).setValidState(ok ? PARAMETER_VALIDATED : PARAMETER_INVALID);
    }
  return ok; // Failed to find parameter
}

/// Check whether a parameter of the given name exists and has an acceptable number of entries.
bool OperatorResult::hasStringParameter(const std::string& pname, int minSize, int maxSize, bool validate) const
{
  bool ok = false;
  Parameters::const_iterator it;
  if ((it = this->m_parameters.find(Parameter(pname))) != this->m_parameters.end())
    {
    int psize = static_cast<int>(it->stringValues().size());
    ok = this->checkParameterSize(psize, minSize, maxSize);
    if (validate)
      const_cast<Parameter&>(*it).setValidState(ok ? PARAMETER_VALIDATED : PARAMETER_INVALID);
    }
  return ok; // Failed to find parameter
}

/// Check whether a parameter of the given name exists and has an acceptable number of entries.
bool OperatorResult::hasIntegerParameter(const std::string& pname, int minSize, int maxSize, bool validate) const
{
  bool ok = false;
  Parameters::const_iterator it;
  if ((it = this->m_parameters.find(Parameter(pname))) != this->m_parameters.end())
    {
    int psize = static_cast<int>(it->integerValues().size());
    ok = this->checkParameterSize(psize, minSize, maxSize);
    if (validate)
      const_cast<Parameter&>(*it).setValidState(ok ? PARAMETER_VALIDATED : PARAMETER_INVALID);
    }
  return ok; // Failed to find parameter
}

/// Check whether a parameter of the given name exists and has an acceptable number of entries.
bool OperatorResult::hasUUIDParameter(const std::string& pname, int minSize, int maxSize, bool validate) const
{
  bool ok = false;
  Parameters::const_iterator it;
  if ((it = this->m_parameters.find(Parameter(pname))) != this->m_parameters.end())
    {
    int psize = static_cast<int>(it->uuidValues().size());
    ok = this->checkParameterSize(psize, minSize, maxSize);
    if (validate)
      const_cast<Parameter&>(*it).setValidState(ok ? PARAMETER_VALIDATED : PARAMETER_INVALID);
    }
  return ok; // Failed to find parameter
}

/// A method used to verify that parameters have the proper number of entries.
bool OperatorResult::checkParameterSize(int psize, int minSize, int maxSize) const
{
  bool ok = false;
  if (minSize < 1)
    {
    ok = maxSize < minSize ?
      true : // Any size OK by caller.
      psize <= maxSize; // only max size specified
    }
  else if (psize >= minSize)
    {
    ok = maxSize < minSize ?
      true : // No maximum
      psize <= maxSize; // both min size and max size checked.
    }
  else
    {
    ok = false; // Failed min size check
    }
  return ok;
}

  } // model namespace
} // smtk namespace
