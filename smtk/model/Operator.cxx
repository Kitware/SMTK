#include "smtk/model/Operator.h"
#include "smtk/model/Manager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/RefItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/util/UUID.h"

#include <sstream>

using smtk::attribute::IntItem;
using smtk::attribute::IntItemPtr;

namespace smtk {
  namespace model {

/// Constructor. Initialize the bridge to a NULL pointer.
Operator::Operator()
{
  this->m_bridge = NULL;
}

/// Destructor. Removes its specification() from the bridge's operator manager.
Operator::~Operator()
{
  if (this->m_bridge)
    {
    if (this->m_specification)
      this->bridge()->operatorManager()->removeAttribute(
        this->m_specification);
    }
}

/**\brief Perform the solid modeling operation the subclass implements.
  *
  * This method first tests whether the operation is well-defined by
  * invoking ableToOperate(). If it returns true, then the
  * operateInternal() method (implemented by subclasses) is invoked.
  *
  * You may register callbacks to observe how the operation is
  * proceeding: you can be signaled when the operation is about
  * to be executed and just after it does execute. Neither will
  * be called if the ableToOperate method returns false.
  */
OperatorResult Operator::operate()
{
  OperatorResult result;
  if (this->ableToOperate())
    {
    if (!this->trigger(WILL_OPERATE))
      result = this->operateInternal();
    else
      result = this->createResult(OPERATION_CANCELED);
    this->trigger(DID_OPERATE, result);
    }
  else
    result = this->createResult(UNABLE_TO_OPERATE);
  return result;
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

/// Return the manager associated with this operator (or a "null"/invalid shared-pointer).
ManagerPtr Operator::manager() const
{
  return this->m_manager;
}

/** Set the manager which initiated the operation.
  *
  * If a Bridge subclass manages transcription for multiple
  * model Manager instances, it is responsible for notifying
  * all of them of any changes.
  * This \a manager is merely the location holding any
  * entities referenced by parameters of the operation.
  *
  * The return value is a shared pointer to this operator.
  */
Operator::Ptr Operator::setManager(ManagerPtr s)
{
  this->m_manager = s;
  return shared_from_this();
}

/// Return the bridge associated with this operator (or a "null"/invalid shared-pointer).
Bridge* Operator::bridge() const
{
  return this->m_bridge;
}

/** Set the bridge that owns this operation.
  *
  * The return value is a shared pointer to this operator.
  */
Operator::Ptr Operator::setBridge(Bridge* b)
{
  this->m_bridge = b;
  return shared_from_this();
}

/**\brief Return the definition of this operation and its parameters.
  *
  * The OperatorDefinition is a typedef to smtk::attribute::Definition
  * so that applications can automatically-generate a user interface
  * for accepting parameter values.
  *
  * However, be aware that the attribute manager used for this
  * specification is owned by the SMTK's model manager and
  * operators are not required to have a valid manager() at all times.
  * This method will return a null pointer if there is no manager.
  * Otherwise, it will ask the bridge and model manager for its
  * definition.
  */
OperatorDefinition Operator::definition() const
{
  Manager::Ptr mgr = this->manager();
  Bridge* brg = this->bridge();
  if (!mgr || !brg)
    return attribute::DefinitionPtr();

  return brg->operatorManager()->findDefinition(this->name());
}

/**\brief Return the specification (if any exists) of this operator.
  *
  * The specification of an operator includes values for each of
  * the operator's parameters as necessary to carry out the operation.
  * These values are encoded in an attribute whose definition is
  * provided by the operator (see smtk::model::Operator::definition()).
  * Note that OperatorSpecification is a typedef of
  * smtk::attribute::AttributePtr.
  *
  * The specification is initially a null attribute pointer.
  * You can create one by calling ensureSpecification().
  * If the operator is invoked without a specification, one
  * is created (holding default values).
  */
OperatorSpecification Operator::specification() const
{
  return this->m_specification;
}

/**\brief Set the specification of the operator's parameters.
  *
  * The attribute, if non-NULL, should match the definition()
  * of the operator.
  */
bool Operator::setSpecification(attribute::AttributePtr spec)
{
  if (spec == this->m_specification)
    return false;

  if (spec)
    if (!spec->isA(this->definition()))
      return false;

  this->m_specification = spec;
  return true;
}

/**\brief Ensure that a specification exists for this operator.
  *
  * Returns true when a specification was created or already existed
  * and false upon error (e.g., when the bridge was not set or
  * no definition exists for this operator's name).
  */
bool Operator::ensureSpecification()
{
  if (this->m_specification)
    return true;

  if (!this->m_bridge)
    return false;

  smtk::attribute::AttributePtr spec =
    this->m_bridge->operatorManager()->createAttribute(this->name());
  if (!spec)
    return false;
  return this->setSpecification(spec);
}

/**\brief Create an attribute representing this operator's result type.
  *
  * The default \a outcome is UNABLE_TO_OPERATE.
  */
OperatorResult Operator::createResult(OperatorOutcome outcome)
{
  std::ostringstream rname;
  rname << "result(" << this->name() << ")";
  OperatorResult result =
    this->bridge()->operatorManager()->createAttribute(rname.str());
  IntItemPtr outcomeItem =
    smtk::dynamic_pointer_cast<IntItem>(
      result->find("outcome"));
  outcomeItem->setValue(outcome);
  return result;
}

/**\brief Remove an attribute from the operator's manager.
  *
  * This is a convenience method to remove an operator's result
  * when you are done examining it.
  *
  * When operating in client-server mode, it is possible for
  * result instances on the client and server to have name
  * collisions unless you manage their lifespans by removing
  * them as they are consumed by your application.
  */
void Operator::eraseResult(OperatorResult res)
{
  Bridge* brdg;
  smtk::attribute::Manager* mgr;
  if (
    !res ||
    !(brdg = this->bridge()) ||
    !(mgr = brdg->operatorManager()))
    return;
  mgr->removeAttribute(res);
}

/// A comparator so that Operators may be placed in ordered sets.
bool Operator::operator < (const Operator& other) const
{
  return this->name() < other.name();
}

std::string outcomeAsString(int oc)
{
  switch (oc)
    {
  case UNABLE_TO_OPERATE:   return "unable to operate";
  case OPERATION_CANCELED:  return "operation canceled";
  case OPERATION_FAILED:    return "operation failed";
  case OPERATION_SUCCEEDED: return "operation succeeded";
  case OUTCOME_UNKNOWN:     break;
    }
  return "outcome unknown";
}

OperatorOutcome stringToOutcome(const std::string& oc)
{
  if (oc == "unable to operate")   return UNABLE_TO_OPERATE;
  if (oc == "operation canceled")  return OPERATION_CANCELED;
  if (oc == "operation failed")    return OPERATION_FAILED;
  if (oc == "operation succeeded") return OPERATION_SUCCEEDED;

  return OUTCOME_UNKNOWN;
}

  } // model namespace
} // smtk namespace
