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

#include "smtk/io/Logger.h"
#include "smtk/io/SaveJSON.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/DoubleItem.h"
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

#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/core/Manager.h"
#include "smtk/mesh/core/MeshSet.h"

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

namespace smtk
{
namespace model
{

/// Constructor. Initialize the session to a NULL pointer.
Operator::Operator()
{
  this->m_debugLevel = 0;
}

/// Destructor. Removes its specification() from the session's operator collection.
Operator::~Operator()
{
  Session::Ptr sess = this->session();
  if (sess && sess->operatorCollection() && this->m_specification)
  {
    sess->operatorCollection()->removeAttribute(this->m_specification);
  }
}

static void markResultModels(
  smtk::attribute::ModelEntityItem::Ptr itm, std::set<smtk::model::Model>& visited, int clean)
{
  if (!itm)
  {
    return;
  }
  std::size_t nn = itm->numberOfValues();
  smtk::model::Model mod;
  for (std::size_t ii = 0; ii < nn; ++ii)
  {
    EntityRef ent = itm->value(ii);
    mod = (ent.isValid() && !ent.isModel() ? mod = ent.owningModel() : ent.as<Model>());
    if (mod.isValid() && visited.find(mod) == visited.end())
    { // model we haven't seen before
      mod.setIntegerProperty("clean", clean);
      visited.insert(mod);
    }
  }
}

// Mark models owning any new/modified entities in the result as either
// "clean" (newly loaded/saved) or "dirty" (modified from file or never written).
static void markResultModels(OperatorResult result)
{
  auto cleanseItem = result->findVoid("cleanse entities");
  int clean = cleanseItem && cleanseItem->isEnabled() ? 1 : 0;
  std::set<smtk::model::Model> visited;
  markResultModels(result->findModelEntity("created"), visited, clean);
  markResultModels(result->findModelEntity("modified"), visited, clean);
  markResultModels(result->findModelEntity("tess_changed"), visited, clean);
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
  // Remember where the log was so we only serialize messages for this operation:
  std::size_t logStart = this->log().numberOfRecords();

  OperatorResult result;
  if (this->ableToOperate())
  {
    // Set the debug level if specified as a convenience for subclasses:
    smtk::attribute::IntItem::Ptr debugItem = this->specification()->findInt("debug level");
    this->m_debugLevel = (debugItem->isEnabled() ? debugItem->value() : 0);
    // Run the operation if possible:
    if (!this->trigger(OperatorEventType::WILL_OPERATE))
      result = this->operateInternal();
    else
      result = this->createResult(OPERATION_CANCELED);
    // Assign names if requested:
    smtk::attribute::IntItem::Ptr assignNamesItem;
    int outcome = result->findInt("outcome")->value();
    if (outcome == OPERATION_SUCCEEDED)
    {
      markResultModels(result);
      if ((assignNamesItem = this->specification()->findInt("assign names")) &&
        assignNamesItem->isEnabled() && assignNamesItem->value() != 0)
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
    }
    this->generateSummary(result);
    // Now grab all log messages and serialize them into the result attribute.
    std::size_t logEnd = this->log().numberOfRecords();
    if (logEnd > logStart)
    { // Serialize relevant log records to JSON.
      cJSON* array = cJSON_CreateArray();
      smtk::io::SaveJSON::forLog(array, this->log(), logStart, logEnd);
      char* logstr = cJSON_Print(array);
      cJSON_Delete(array);
      result->findString("log")->appendValue(logstr);
      free(logstr);
    }
    // Inform observers that the operation completed.
    this->trigger(OperatorEventType::DID_OPERATE, result);

    smtk::attribute::ModelEntityItem::Ptr tess_changed = result->findModelEntity("tess_changed");
    if (tess_changed)
    {
      for (auto it = tess_changed->begin(); it != tess_changed->end(); ++it)
      {
        smtk::mesh::CollectionPtr collection =
          this->manager()->meshes()->collection(it->owningModel().entity());
        if (collection && collection->isValid())
        {
          smtk::mesh::MeshSet modified = collection->findAssociatedMeshes(*it);
          if (!modified.is_empty())
          {
            collection->removeMeshes(modified);
          }
        }
      }
    }
  }
  else
  {
    // Do not inform observers since this is currently a non-event.
    result = this->createResult(UNABLE_TO_OPERATE);
    // Now grab all log messages and serialize them into the result attribute.
    std::size_t logEnd = this->log().numberOfRecords();
    if (logEnd > logStart)
    { // Serialize relevant log records to JSON.
      cJSON* array = cJSON_CreateArray();
      smtk::io::SaveJSON::forLog(array, this->log(), logStart, logEnd);
      char* logstr = cJSON_Print(array);
      cJSON_Delete(array);
      result->findString("log")->appendValue(logstr);
      free(logstr);
    }
  }
  return result;
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
SessionPtr Operator::session() const
{
  return this->m_session.lock();
}

/**\brief Set the session that owns this operation.
  *
  * The return value is a shared pointer to this operator.
  */
Operator::Ptr Operator::setSession(SessionPtr b)
{
  this->m_session = b;
  return this->shared_from_this();
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
  SessionPtr brg = this->session();
  if (!mgr || !brg)
    return attribute::DefinitionPtr();

  return brg->operatorCollection()->findDefinition(this->name());
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

  SessionPtr sess = this->session(); // Lock the session
  if (!sess)
  {
    return false;
  }

  smtk::attribute::AttributePtr spec = sess->operatorCollection()->createAttribute(this->name());
  if (!spec)
  {
    return false;
  }
  return const_cast<Operator*>(this)->setSpecification(spec);
}

/**\brief Parameter and association convenience methods.
  *
  */
///@{

/// Return the integer-valued parameter named \a name or NULL if it does not exist.
smtk::attribute::IntItemPtr Operator::findInt(
  const std::string& pname, smtk::attribute::SearchStyle search)
{
  return this->specification()->findAs<IntItem>(pname, search);
}

/// Return the integer-valued parameter named \a name or NULL if it does not exist.
smtk::attribute::DoubleItemPtr Operator::findDouble(
  const std::string& pname, smtk::attribute::SearchStyle search)
{
  return this->specification()->findAs<DoubleItem>(pname, search);
}

/// Return the integer-valued parameter named \a name or NULL if it does not exist.
smtk::attribute::StringItemPtr Operator::findString(
  const std::string& pname, smtk::attribute::SearchStyle search)
{
  return this->specification()->findAs<StringItem>(pname, search);
}

/// Return the integer-valued parameter named \a name or NULL if it does not exist.
smtk::attribute::FileItemPtr Operator::findFile(
  const std::string& pname, smtk::attribute::SearchStyle search)
{
  return this->specification()->findAs<FileItem>(pname, search);
}

/// Return the integer-valued parameter named \a name or NULL if it does not exist.
smtk::attribute::DirectoryItemPtr Operator::findDirectory(
  const std::string& pname, smtk::attribute::SearchStyle search)
{
  return this->specification()->findAs<DirectoryItem>(pname, search);
}

/// Return the integer-valued parameter named \a name or NULL if it does not exist.
smtk::attribute::GroupItemPtr Operator::findGroup(
  const std::string& pname, smtk::attribute::SearchStyle search)
{
  return this->specification()->findAs<GroupItem>(pname, search);
}

/// Return the integer-valued parameter named \a name or NULL if it does not exist.
smtk::attribute::RefItemPtr Operator::findRef(
  const std::string& pname, smtk::attribute::SearchStyle search)
{
  return this->specification()->findAs<RefItem>(pname, search);
}

/// Return the integer-valued parameter named \a name or NULL if it does not exist.
smtk::attribute::ModelEntityItemPtr Operator::findModelEntity(
  const std::string& pname, smtk::attribute::SearchStyle search)
{
  return this->specification()->findAs<ModelEntityItem>(pname, search);
}

/// Return the integer-valued parameter named \a name or NULL if it does not exist.
smtk::attribute::VoidItemPtr Operator::findVoid(
  const std::string& pname, smtk::attribute::SearchStyle search)
{
  return this->specification()->findAs<VoidItem>(pname, search);
}

/// Return the mesh-selection-item parameter named \a name or NULL if it does not exist.
smtk::attribute::MeshSelectionItemPtr Operator::findMeshSelection(
  const std::string& pname, smtk::attribute::SearchStyle search)
{
  return this->specification()->findAs<MeshSelectionItem>(pname, search);
}

/// Return the mesh-entity-item parameter named \a name or NULL if it does not exist.
smtk::attribute::MeshItemPtr Operator::findMesh(
  const std::string& pname, smtk::attribute::SearchStyle search)
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
  OperatorResult result = this->session()->operatorCollection()->createAttribute(rname.str());
  IntItemPtr outcomeItem = smtk::dynamic_pointer_cast<IntItem>(result->find("outcome"));
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
  SessionPtr brdg;
  smtk::attribute::CollectionPtr sys;
  if (!res || !(brdg = this->session()) || !(sys = brdg->operatorCollection()))
    return;
  sys->removeAttribute(res);
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
  EntityRefArray tmp(1, ent);
  this->addEntitiesToResult(res, tmp, gen);
}

} // model namespace
} // smtk namespace
