//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/Operator.h"
#include "smtk/model/Manager.h"

#include "smtk/io/ExportJSON.h"
#include "smtk/io/Logger.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/MeshItem.h"
#include "smtk/attribute/MeshSelectionItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/RefItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/common/UUID.h"

#include "cJSON.h"

#include <sstream>

using smtk::attribute::IntItem;
using smtk::attribute::IntItemPtr;
using smtk::attribute::DoubleItem;
using smtk::attribute::StringItem;
using smtk::attribute::FileItem;
using smtk::attribute::DirectoryItem;
using smtk::attribute::GroupItem;
using smtk::attribute::RefItem;
using smtk::attribute::ModelEntityItem;
using smtk::attribute::MeshSelectionItem;
using smtk::attribute::MeshItem;
using smtk::attribute::VoidItem;

namespace smtk {
  namespace model {

/// Constructor. Initialize the session to a NULL pointer.
Operator::Operator()
{
  this->m_session = NULL;
  this->m_debugLevel = 0;
}

/// Destructor. Removes its specification() from the session's operator system.
Operator::~Operator()
{
  if (this->m_session  &&
      this->m_session->operatorSystem() &&
      this->m_specification)
    {
    this->m_session->operatorSystem()->removeAttribute(
      this->m_specification);
    }
}

/**\brief Return whether the operator's inputs are well-defined.
  *
  * This returns true when the Operator considers its inputs to
  * be valid and false otherwise.
  *
  * Subclasses may override this method.
  * By default, it returns true when this->specification()->isValid()
  * returns true.
  */
bool Operator::ableToOperate()
{
  return this->specification()->isValid();
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
    // Remember where the log was so we only serialize messages for this operation:
    std::size_t logStart = this->log().numberOfRecords();
    // Set the debug level if specified as a convenience for subclasses:
    smtk::attribute::IntItem::Ptr debugItem = this->specification()->findInt("debug level");
    this->m_debugLevel = (debugItem->isEnabled() ? debugItem->value() : 0);
    // Run the operation if possible:
    if (!this->trigger(WILL_OPERATE))
      result = this->operateInternal();
    else
      result = this->createResult(OPERATION_CANCELED);
    // Assign names if requested:
    smtk::attribute::IntItem::Ptr assignNamesItem;
    if (
      result->findInt("outcome")->value() == OPERATION_SUCCEEDED &&
      (assignNamesItem = this->specification()->findInt("assign names")) &&
      assignNamesItem->isEnabled() &&
      assignNamesItem->value() != 0)
      {
      ModelEntityItem::Ptr thingsToName = result->findModelEntity("created");
      EntityRefArray::const_iterator it;
      for (it = thingsToName->begin(); it != thingsToName->end(); ++it)
        {
        Model model(*it);
        if (model.isValid())
          model.assignDefaultNames();
        }
      }
    this->generateSummary(result);
    // Now grab all log messages and serialize them into the result attribute.
    std::size_t logEnd = this->log().numberOfRecords();
    if (logEnd > logStart)
      { // Serialize relevant log records to JSON.
      cJSON* array = cJSON_CreateArray();
      smtk::io::ExportJSON::forLog(array, this->log(), logStart, logEnd);
      char* logstr = cJSON_Print(array);
      cJSON_Delete(array);
      result->findString("log")->appendValue(logstr);
      free(logstr);
      }
    // Inform observers that the operation completed.
    this->trigger(DID_OPERATE, result);
    }
  else
    {
    // Do not inform observers since this is currently a non-event.
    result = this->createResult(UNABLE_TO_OPERATE);
    }
  return result;
}

/// Add an observer of WILL_OPERATE events on this operator.
void Operator::observe(OperatorEventType event, BareOperatorCallback functionHandle, void* callData)
{
  (void)event;
  this->m_willOperateTriggers.insert(
    std::make_pair(functionHandle, callData));
}

/// Add an observer of DID_OPERATE events on this operator.
void Operator::observe(OperatorEventType event, OperatorWithResultCallback functionHandle, void* callData)
{
  (void)event;
  this->m_didOperateTriggers.insert(
    std::make_pair(functionHandle, callData));
}

/// Remove an existing WILL_OPERATE observer. The \a callData must match the value passed to Operator::observe().
void Operator::unobserve(OperatorEventType event, BareOperatorCallback functionHandle, void* callData)
{
  (void)event;
  this->m_willOperateTriggers.erase(
    std::make_pair(functionHandle, callData));
}

/// Remove an existing DID_OPERATE observer. The \a callData must match the value passed to Operator::observe().
void Operator::unobserve(OperatorEventType event, OperatorWithResultCallback functionHandle, void* callData)
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
  std::set<BareOperatorObserver>::const_iterator it;
  for (it = this->m_willOperateTriggers.begin(); it != this->m_willOperateTriggers.end(); ++it)
    status |= (*it->first)(event, *this, it->second);
  return status;
}

/// Invoke all DID_OPERATE observer callbacks. The return value is always 0 (this may change in future releases).
int Operator::trigger(OperatorEventType event, const OperatorResult& result)
{
  std::set<OperatorWithResultObserver>::const_iterator it;
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
  * If a Session subclass manages transcription for multiple
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

/// Return the meshManager associated with this operator (or a "null"/invalid shared-pointer).
smtk::mesh::ManagerPtr Operator::meshManager() const
{
  return this->m_meshmanager;
}

/** Set the meshManager associated with session that initiated the operation.
  *
  *
  * The return value is a shared pointer to this operator.
  */
Operator::Ptr Operator::setMeshManager(smtk::mesh::ManagerPtr s)
{
  this->m_meshmanager = s;
  return shared_from_this();
}

/// Return the session associated with this operator (or a "null"/invalid shared-pointer).
Session* Operator::session() const
{
  return this->m_session;
}

/**\brief Set the session that owns this operation.
  *
  * The return value is a shared pointer to this operator.
  */
Operator::Ptr Operator::setSession(Session* b)
{
  this->m_session = b;
  return shared_from_this();
}

/**\brief A convenience method to return the manager's log.
  *
  * Always use the log for errors and informational messages
  * so that there is a chance that users will see them.
  */
smtk::io::Logger& Operator::log()
{
  static smtk::io::Logger dummy;
  return this->manager() ? this->manager()->log() : dummy;
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
  * Otherwise, it will ask the session and model manager for its
  * definition.
  */
OperatorDefinition Operator::definition() const
{
  Manager::Ptr mgr = this->manager();
  Session* brg = this->session();
  if (!mgr || !brg)
    return attribute::DefinitionPtr();

  return brg->operatorSystem()->findDefinition(this->name());
}

/**\brief Return the specification of this operator (creating one if none exists).
  *
  * The specification of an operator includes values for each of
  * the operator's parameters as necessary to carry out the operation.
  * These values are encoded in an attribute whose definition is
  * provided by the operator (see smtk::model::Operator::definition()).
  * Note that OperatorSpecification is a typedef of
  * smtk::attribute::AttributePtr.
  *
  * The specification is initially a null attribute pointer
  * but is initialized when calling this method or
  * by calling ensureSpecification().
  *
  * If the operator is invoked without a specification, one
  * is created (holding default values).
  */
OperatorSpecification Operator::specification() const
{
  this->ensureSpecification();
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
  * and false upon error (e.g., when the session was not set or
  * no definition exists for this operator's name).
  */
bool Operator::ensureSpecification() const
{
  if (this->m_specification)
    return true;

  if (!this->m_session)
    return false;

  smtk::attribute::AttributePtr spec =
    this->m_session->operatorSystem()->createAttribute(this->name());
  if (!spec)
    return false;
  return const_cast<Operator*>(this)->setSpecification(spec);
}

/**\brief Parameter and association convenience methods.
  *
  */
///@{

/// Return the integer-valued parameter named \a name or NULL if it does not exist.
smtk::attribute::IntItemPtr Operator::findInt(const std::string& pname, smtk::attribute::SearchStyle search)
{
  return this->specification()->findAs<IntItem>(pname, search);
}

/// Return the integer-valued parameter named \a name or NULL if it does not exist.
smtk::attribute::DoubleItemPtr Operator::findDouble(const std::string& pname, smtk::attribute::SearchStyle search)
{
  return this->specification()->findAs<DoubleItem>(pname, search);
}

/// Return the integer-valued parameter named \a name or NULL if it does not exist.
smtk::attribute::StringItemPtr Operator::findString(const std::string& pname, smtk::attribute::SearchStyle search)
{
  return this->specification()->findAs<StringItem>(pname, search);
}

/// Return the integer-valued parameter named \a name or NULL if it does not exist.
smtk::attribute::FileItemPtr Operator::findFile(const std::string& pname, smtk::attribute::SearchStyle search)
{
  return this->specification()->findAs<FileItem>(pname, search);
}

/// Return the integer-valued parameter named \a name or NULL if it does not exist.
smtk::attribute::DirectoryItemPtr Operator::findDirectory(const std::string& pname, smtk::attribute::SearchStyle search)
{
  return this->specification()->findAs<DirectoryItem>(pname, search);
}

/// Return the integer-valued parameter named \a name or NULL if it does not exist.
smtk::attribute::GroupItemPtr Operator::findGroup(const std::string& pname, smtk::attribute::SearchStyle search)
{
  return this->specification()->findAs<GroupItem>(pname, search);
}

/// Return the integer-valued parameter named \a name or NULL if it does not exist.
smtk::attribute::RefItemPtr Operator::findRef(const std::string& pname, smtk::attribute::SearchStyle search)
{
  return this->specification()->findAs<RefItem>(pname, search);
}

/// Return the integer-valued parameter named \a name or NULL if it does not exist.
smtk::attribute::ModelEntityItemPtr Operator::findModelEntity(const std::string& pname, smtk::attribute::SearchStyle search)
{
  return this->specification()->findAs<ModelEntityItem>(pname, search);
}

/// Return the integer-valued parameter named \a name or NULL if it does not exist.
smtk::attribute::VoidItemPtr Operator::findVoid(const std::string& pname, smtk::attribute::SearchStyle search)
{
  return this->specification()->findAs<VoidItem>(pname, search);
}

/// Return the mesh-selection-item parameter named \a name or NULL if it does not exist.
smtk::attribute::MeshSelectionItemPtr Operator::findMeshSelection(const std::string& pname, smtk::attribute::SearchStyle search)
{
  return this->specification()->findAs<MeshSelectionItem>(pname, search);
}

/// Return the mesh-entity-item parameter named \a name or NULL if it does not exist.
smtk::attribute::MeshItemPtr Operator::findMesh(const std::string& pname, smtk::attribute::SearchStyle search)
{
  return this->specification()->findAs<MeshItem>(pname, search);
}

/// Associate a model entity with the operator.
bool Operator::associateEntity(const smtk::model::EntityRef& entity)
{
  return this->specification()->associateEntity(entity);
}

/// Disassociate a model entity with the operator.
void Operator::disassociateEntity(const smtk::model::EntityRef& entity)
{
  this->specification()->disassociateEntity(entity);
}

/// Disassociate all model entities from the operator.
void Operator::removeAllAssociations()
{
  this->specification()->removeAllAssociations();
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
    this->session()->operatorSystem()->createAttribute(rname.str());
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
  Session* brdg;
  smtk::attribute::System* sys;
  if (
    !res ||
    !(brdg = this->session()) ||
    !(sys = brdg->operatorSystem()))
    return;
  sys->removeAttribute(res);
}

/// A comparator so that Operators may be placed in ordered sets.
bool Operator::operator < (const Operator& other) const
{
  return this->name() < other.name();
}

void Operator::generateSummary(OperatorResult& res)
{
  std::ostringstream msg;
  int outcome = res->findInt("outcome")->value();
  msg << this->specification()->definition()->label() << ": " << outcomeAsString(outcome);
  if (outcome == static_cast<int>(OPERATION_SUCCEEDED))
    {
    smtkInfoMacro(this->log(), msg.str());
    }
  else
    {
    smtkErrorMacro(this->log(), msg.str());
    }
}

/// Return a string summarizing the outcome of an operation.
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

/// Given a string summarizing the outcome of an operation, return an enumerant.
OperatorOutcome stringToOutcome(const std::string& oc)
{
  if (oc == "unable to operate")   return UNABLE_TO_OPERATE;
  if (oc == "operation canceled")  return OPERATION_CANCELED;
  if (oc == "operation failed")    return OPERATION_FAILED;
  if (oc == "operation succeeded") return OPERATION_SUCCEEDED;

  return OUTCOME_UNKNOWN;
}

/*! \fn Operator::operateInternal()
 * \brief Perform the requested operation on this operator's specification.
 *
 * Subclasses must implement this method.
 */

/**\brief Add an entity to an operator's result attribute.
  *
  * See Operator::addEntitiesToResult() for details.
  */
void Operator::addEntityToResult(OperatorResult res, const EntityRef& ent, ResultEntityOrigin gen)
{
  EntityRefArray tmp(1,ent);
  this->addEntitiesToResult(res, tmp, gen);
}

  } // model namespace
} // smtk namespace
