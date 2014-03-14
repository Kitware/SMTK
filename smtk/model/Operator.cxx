#include "smtk/model/Operator.h"
#include "smtk/model/OperatorResult.h"
#include "smtk/model/Parameter.h"
#include "smtk/model/Storage.h"

namespace smtk {
  namespace model {

OperatorResult Operator::operate()
{
  OperatorResult result(UNABLE_TO_OPERATE); // state = could not operate
  if (this->ableToOperate())
    {
    result = OperatorResult(OPERATION_CANCELED);
    if (!this->trigger(WILL_OPERATE))
      {
      result = this->operateInternal();
      }
    this->trigger(DID_OPERATE, result);
    }
  return result;
}

/// Return a copy of this operator's parameters.
Parameters Operator::parameters() const
{
  return this->m_parameters;
}

/// Return the parameter of the given name (or an invalid, uninitialized parameter if it does not exist)
const Parameter& Operator::parameter(const std::string& name) const
{
  static const Parameter invalid;
  Parameters::const_iterator it;
  if ((it = this->m_parameters.find(Parameter(name))) != this->m_parameters.end())
    return *it;
  return invalid;
}

/// Return the parameter of the given name (or an invalid, uninitialized parameter if it does not exist)
Parameter Operator::parameter(const std::string& name)
{
  Parameters::const_iterator it;
  if ((it = this->m_parameters.find(Parameter(name))) != this->m_parameters.end())
    return *it;
  return Parameter();
}


/// Set the value of a parameter to \a p. Note that unexpected parameters will be ignored by most operators.
void Operator::setParameter(const Parameter& p)
{
  Parameters::const_iterator old = this->m_parameters.find(p);
  if (old != this->m_parameters.end())
    {
    this->trigger(PARAMETER_CHANGE, *old, p);
    this->m_parameters.erase(*old);
    }
  else
    {
    this->trigger(PARAMETER_CHANGE, Parameter(), p);
    }
  this->m_parameters.insert(p);
}

/// Check whether a parameter of the given name exists and has an acceptable number of entries.
bool Operator::hasFloatParameter(const std::string& name, int minSize, int maxSize, bool validate)
{
  bool ok = false;
  Parameters::const_iterator it;
  if ((it = this->m_parameters.find(Parameter(name))) != this->m_parameters.end())
    {
    int psize = static_cast<int>(it->floatValues().size());
    ok = this->checkParameterSize(psize, minSize, maxSize);
    if (validate)
      const_cast<Parameter&>(*it).setValidState(ok ? PARAMETER_VALIDATED : PARAMETER_INVALID);
    }
  return ok; // Failed to find parameter
}

/// Check whether a parameter of the given name exists and has an acceptable number of entries.
bool Operator::hasStringParameter(const std::string& name, int minSize, int maxSize, bool validate)
{
  bool ok = false;
  Parameters::const_iterator it;
  if ((it = this->m_parameters.find(Parameter(name))) != this->m_parameters.end())
    {
    int psize = static_cast<int>(it->stringValues().size());
    ok = this->checkParameterSize(psize, minSize, maxSize);
    if (validate)
      const_cast<Parameter&>(*it).setValidState(ok ? PARAMETER_VALIDATED : PARAMETER_INVALID);
    }
  return ok; // Failed to find parameter
}

/// Check whether a parameter of the given name exists and has an acceptable number of entries.
bool Operator::hasIntegerParameter(const std::string& name, int minSize, int maxSize, bool validate)
{
  bool ok = false;
  Parameters::const_iterator it;
  if ((it = this->m_parameters.find(Parameter(name))) != this->m_parameters.end())
    {
    int psize = static_cast<int>(it->integerValues().size());
    ok = this->checkParameterSize(psize, minSize, maxSize);
    if (validate)
      const_cast<Parameter&>(*it).setValidState(ok ? PARAMETER_VALIDATED : PARAMETER_INVALID);
    }
  return ok; // Failed to find parameter
}

/// Check whether a parameter of the given name exists and has an acceptable number of entries.
bool Operator::hasUUIDParameter(const std::string& name, int minSize, int maxSize, bool validate)
{
  bool ok = false;
  Parameters::const_iterator it;
  if ((it = this->m_parameters.find(Parameter(name))) != this->m_parameters.end())
    {
    int psize = static_cast<int>(it->uuidValues().size());
    ok = this->checkParameterSize(psize, minSize, maxSize);
    if (validate)
      const_cast<Parameter&>(*it).setValidState(ok ? PARAMETER_VALIDATED : PARAMETER_INVALID);
    }
  return ok; // Failed to find parameter
}

/// Add an observer to parameter changes of this operator.
void Operator::observe(OperatorEventType event, ParameterChangeCallback functionHandle, void* callData)
{
  (void)event;
  this->m_parameterChangeTriggers.insert(
    std::make_pair(functionHandle, callData));
}

/// Add an observer of WILL_OPERATE events on this operator.
void Operator::observe(OperatorEventType event, WillOperateCallback functionHandle, void* callData)
{
  (void)event;
  this->m_willOperateTriggers.insert(
    std::make_pair(functionHandle, callData));
}

/// Add an observer of DID_OPERATE events on this operator.
void Operator::observe(OperatorEventType event, DidOperateCallback functionHandle, void* callData)
{
  (void)event;
  this->m_didOperateTriggers.insert(
    std::make_pair(functionHandle, callData));
}

/// Remove an existing parameter-change observer. The \a callData must match the value passed to Operator::observe().
void Operator::unobserve(OperatorEventType event, ParameterChangeCallback functionHandle, void* callData)
{
  (void)event;
  this->m_parameterChangeTriggers.erase(
    std::make_pair(functionHandle, callData));
}

/// Remove an existing WILL_OPERATE observer. The \a callData must match the value passed to Operator::observe().
void Operator::unobserve(OperatorEventType event, WillOperateCallback functionHandle, void* callData)
{
  (void)event;
  this->m_willOperateTriggers.erase(
    std::make_pair(functionHandle, callData));
}

/// Remove an existing DID_OPERATE observer. The \a callData must match the value passed to Operator::observe().
void Operator::unobserve(OperatorEventType event, DidOperateCallback functionHandle, void* callData)
{
  (void)event;
  this->m_didOperateTriggers.erase(
    std::make_pair(functionHandle, callData));
}

/// Invoke all parameter-change observer callbacks. The return value is always 0 (this may change in future releases).
int Operator::trigger(OperatorEventType event, const Parameter& oldVal, const Parameter& newVal)
{
  std::set<ParameterChangeObserver>::const_iterator it;
  for (it = this->m_parameterChangeTriggers.begin(); it != this->m_parameterChangeTriggers.end(); ++it)
    (*it->first)(event, *this, oldVal, newVal, it->second);
  return 0;
}

/**\brief Invoke all WILL_OPERATE observer callbacks.
  *
  * The return value is non-zero if the operation was canceled and zero otherwise.
  * Note that all observers will be called even if one requests the operation be
  * canceled. This is useful since all DID_OPERATE observers are called whether
  * the operation was canceled or not -- and observers of both will expect them
  * to be called in pairs.
  */
int Operator::trigger(OperatorEventType event)
{
  int status = 0;
  std::set<WillOperateObserver>::const_iterator it;
  for (it = this->m_willOperateTriggers.begin(); it != this->m_willOperateTriggers.end(); ++it)
    status |= (*it->first)(event, *this, it->second);
  return status;
}

/// Invoke all DID_OPERATE observer callbacks. The return value is always 0 (this may change in future releases).
int Operator::trigger(OperatorEventType event, const OperatorResult& result)
{
  std::set<DidOperateObserver>::const_iterator it;
  for (it = this->m_didOperateTriggers.begin(); it != this->m_didOperateTriggers.end(); ++it)
    (*it->first)(event, *this, result, it->second);
  return 0;
}

/// Return the storage associated with this operator (or a "null"/invalid shared-pointer).
StoragePtr Operator::storage() const
{
  return this->m_storage;
}

/** Set the storage which will initiate the operation.
  *
  * If a BridgeBase subclass manages multiple Storage instances,
  * it is responsible for notifying all of them of any changes.
  * This \a storage is merely the location holding any
  * entities referenced by parameters of the operation.
  *
  * The return value is a shared pointer to this operator.
  */
Operator::Ptr Operator::setStorage(StoragePtr storage)
{
  this->m_storage = storage;
  return shared_from_this();
}

/// A comparator so that Operators may be placed in ordered sets.
bool Operator::operator < (const Operator& other) const
{
  return this->name() < other.name();
}

/**\brief Copy information from another Operator instance.
  *
  * Subclasses should override this method if they
  * contain any non-static members.
  *
  * The return value is a shared pointer to this instance.
  */
Operator::Ptr Operator::cloneInternal(ConstPtr src)
{
  this->m_parameters = src->parameters();
  this->m_storage = src->storage();
  return shared_from_this();
}

/// A method used to verify that parameters have the proper number of entries.
bool Operator::checkParameterSize(int psize, int minSize, int maxSize)
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

/*! \fn Operator::clone() const
 * \brief Create a copy of this Operator.
 *
 * Subclasses must implement this method.
 * Inside their implementation, they should call
 * Operator::cloneInternal to copy base-class data.
 * If your subclass uses smtkCreateMacro and has no
 * storage, then this implementation should suffice:
 * <pre>
 *   virtual Operator::Ptr clone() const
 *     { return create()->cloneInternal(shared_from_this()); }
 * </pre>
 */

  } // model namespace
} // smtk namespace
