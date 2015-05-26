//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/Session.h"

#include "smtk/model/SessionIO.h"
#include "smtk/model/Manager.h"
#include "smtk/model/RemoteOperator.h"
#include "smtk/model/ArrangementHelper.h"

#include "smtk/model/Model.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/ShellEntity.h"
#include "smtk/model/UseEntity.h"
#include "smtk/model/Instance.h"
#include "smtk/model/Group.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/IntItemDefinition.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"
#include "smtk/attribute/StringItemDefinition.h"
#include "smtk/attribute/System.h"

#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"

using smtk::attribute::Definition;
using smtk::attribute::DefinitionPtr;
using smtk::attribute::IntItemDefinition;
using smtk::attribute::ModelEntityItemDefinition;
using smtk::attribute::StringItemDefinition;

namespace smtk {
  namespace model {

/// Default constructor. This assigns a random session ID to each Session instance.
Session::Session()
  : m_sessionId(smtk::common::UUID::random()), m_operatorSys(NULL)
{
  this->initializeOperatorSystem(Session::s_operators);
}

/// Destructor. We must delete the attribute system that tracks operator definitions.
Session::~Session()
{
  delete this->m_operatorSys;
}

/**\brief Return the name of the session type (i.e., the name of the modeling kernel).
  *
  * Subclasses override this method by using the smtkDeclareModelingKernel
  * and smtkImplementsModelingKernel macros.
  */
std::string Session::name() const
{
  return "invalid";
}

/**\brief Return the session ID for this instance of the session.
  *
  * Sessions are ephemeral and tied to a particular machine so
  * they should generally not be serialized. However, when using
  * JSON stringifications of operators to perform remote procedure
  * calls (RPC), the session ID specifies which Session on which
  * machine should actually invoke the operator.
  */
smtk::common::UUID Session::sessionId() const
{
  return this->m_sessionId;
}

/**\brief Transcribe an entity from a foreign modeler into an SMTK storage Manager.
  *
  * On input, the \a entity will not be valid but if transcription is
  * successful, the \a requested records in the \a entity's Manager will
  * be valid. If \a requested includes SESSION_ENTITY_TYPE, then
  * \a entity.isValid() should return true after this call.
  *
  * Only honor requests for entity IDs listed as dangling unless
  * \a onlyDangling is false (default is true).
  * This prevents expensive requests by Manager instances over many Sessions.
  *
  * The return value is 0 upon failure and non-zero upon success.
  * Failure occurs when any \a requested bits of information that
  * are in Sessione::allSupportedInformation() are not transcribed,
  * or when \a requested is 0.
  */
int Session::transcribe(
  const EntityRef& entity, SessionInfoBits requested, bool onlyDangling, int depth)
{
  int retval = 0;
  if (requested)
    {
    // Check that the entity ID is dangling or we are forced to continue.
    DanglingEntities::iterator it = this->m_dangling.find(entity);
    if (onlyDangling && it == this->m_dangling.end())
      { // The session has not been told that this UUID exists.
      return retval;
      }
    // Ask the subclass to transcribe information.
    SessionInfoBits actual = this->transcribeInternal(entity, requested, depth);
    // Decide which bits of the request can possibly be honored...
    SessionInfoBits honorable = requested & this->allSupportedInformation();
    // ... and verify that all of those have been satisfied.
    retval = (honorable & actual) == honorable;
    // If transcription is complete, then remove the UUID from the dangling
    // entity set. Note that we must refresh the iterator since transcribeInternal
    // may have modified m_dangling.
    if (
      ((actual & this->allSupportedInformation()) == this->allSupportedInformation()) &&
      ((it = this->m_dangling.find(entity)) != this->m_dangling.end()))
        this->m_dangling.erase(it);
    }
  return retval;
}

/**\brief Return a bit vector describing what types of information can be transcribed.
  *
  * This is used to determine when an entity has been fully transcribed into storage
  * and is no longer "dangling."
  */
SessionInfoBits Session::allSupportedInformation() const
{
  return SESSION_EVERYTHING;
}

/// Return a list of names of solid-model operators available.
StringList Session::operatorNames(bool includeAdvanced) const
{
  std::vector<smtk::attribute::DefinitionPtr> ops;
  this->m_operatorSys->derivedDefinitions(
    this->m_operatorSys->findDefinition("operator"), ops);

  StringList nameList;
  std::vector<smtk::attribute::DefinitionPtr>::iterator it;
  for (it = ops.begin(); it != ops.end(); ++it)
    {
    // only show operators that are not advanced
    if (!includeAdvanced && (*it)->advanceLevel() > 0)
      continue;
    nameList.push_back((*it)->type());
    }
  return nameList;
}

OperatorPtr Session::op(const std::string& opName) const
{
  OperatorPtr oper;
  if (opName.empty())
    return oper;

  OperatorConstructor ctor = this->findOperatorConstructor(opName);
  if (!ctor)
    return oper;

  oper = ctor();
  if (!oper)
    return oper;

  oper->setSession(const_cast<Session*>(this));
  oper->setManager(this->manager());

  RemoteOperator::Ptr remoteOp = smtk::dynamic_pointer_cast<RemoteOperator>(oper);
  if (remoteOp)
    remoteOp->setName(opName);

  return oper;
}

/// Return the map from dangling entityrefs to bits describing their partial transcription state.
const DanglingEntities& Session::danglingEntities() const
{
  return this->m_dangling;
}

/**\brief Mark an entity, \a ent, as partially transcribed.
  *
  * Subclasses should call this method when a UUID has been assigned
  * to a model entity but ent.manager() has not yet been populated with
  * all of the information about the entity. The information which *is*
  * \a present in ent.manager() should be passed but will default to
  * zero (i.e., the UUID exists in some other entity's relations but
  * has no records in manager itself).
  *
  * The entity is added to the list of dangling entities and will be
  * removed from the list when a call to \a transcribeInternal indicates
  * that Session::allSupportedInformation() is now present in manager.
  */
void Session::declareDanglingEntity(const EntityRef& ent, SessionInfoBits present)
{
  if ((present & this->allSupportedInformation()) < this->allSupportedInformation())
    this->m_dangling[ent] = present;
  else
    this->m_dangling.erase(ent);
}

/** @name Operator Manager
  *\brief Return this session's internal attribute system, used to describe operators.
  *
  * Each operator should have a definition of the same name held in this manager.
  */
///@{
smtk::attribute::System* Session::operatorSystem()
{
  return this->m_operatorSys;
}

const smtk::attribute::System* Session::operatorSystem() const
{
  return this->m_operatorSys;
}
///@}

/**\brief Set configuration options on the session.
  *
  * Subclasses may override this method to accept configuration
  * options specific to their backends.
  * When \a optName and \a optVal are acceptable, the
  * method returns 1; otherwise a zero or negative value is returned.
  */
int Session::setup(const std::string& optName, const StringList& optVal)
{
  (void)optName;
  (void)optVal;
  return 0;
}

/// Return a reference to the manager that owns this Session.
Manager::Ptr Session::manager() const
{
  return this->m_manager ?
    this->m_manager->shared_from_this() :
    Manager::Ptr();
}

/// Return the log (obtained from the model manager).
smtk::io::Logger& Session::log()
{
  return this->manager()->log();
}

/**\brief Transcribe information requested by \a flags into \a entity from foreign modeler.
  *
  * This method should return a non-zero value upon success.
  * Upon success, \a flags should be modified to represent the
  * actual information transcribed (as opposed to what was requested).
  * This should always be at least the information requested but may
  * include more information.
  *
  * Currently, it really only makes sense to call this method on a
  * Model (i.e., not an edge, face, etc.); entire models at a time
  * are retranscribed.
  *
  * Subclasses may override this method.
  * If they do not, they should implement the virtual relationship helper methods.
  */
SessionInfoBits Session::transcribeInternal(const EntityRef& entRef, SessionInfoBits flags, int depth)
{
  (void)depth;
  SessionInfoBits actual = SESSION_NOTHING;
  Entity* entRec = this->m_manager->findEntity(entRef.entity(), false);
  if (!entRec)
    entRec = this->addEntityRecord(entRef);

  // Get a subclass-specific helper for validating/repairing/creating arrangements
  ArrangementHelper* helper = this->createArrangementHelper();

  // Now recursively find all related entities.
  // This marks entRef, resets it (removing all relations), and stores state in the helper
  // as required to re-transcribe the state in a manner as consistent with the previous state
  // as possible. (For example the helper might store the sense number of an edge or vertex
  // with respect to its parent face or edge so that retranscription results in the same
  // senses if possible.)
  this->findOrAddRelatedEntities(entRef, flags, helper);
  helper->doneAddingEntities(this->shared_from_this(), flags);

  // We must re-find entRec because the addition of other entities may
  // have caused a reallocation (in hash-based storage):
  entRec = this->m_manager->findEntity(entRef.entity(), false);

  actual |= this->findOrAddArrangements(entRef, entRec, flags, helper);
  actual |= this->updateProperties(entRef, entRec, flags, helper);
  actual |= this->updateTessellation(entRef, flags, helper);
  delete helper;

  // Return what we actually transcribed so that parent can update
  // the dangling entity map.
  return actual;
}

/**\brief Set the session ID.
  *
  * Do not call this unless you are preparing the session
  * to be a remote mirror of a modeling session (for, e.g.,
  * client-server operation).
  */
void Session::setSessionId(const smtk::common::UUID& sessId)
{
  this->m_sessionId = sessId;
}

/// Inform this instance of the session that it is owned by \a mgr.
void Session::setManager(Manager* mgr)
{
  this->m_manager = mgr;
  this->m_operatorSys->setRefModelManager(
    mgr->shared_from_this());
}

/**\brief This is used by the manager when erasing a model entity.
  *
  * Subclasses should implement this and erase all of the string, integer,
  * and floating-point properties (as specified by \a propFlags) that
  * their modeling kernel allows them to reproduce when transcribe() is
  * called.
  * The properties should only be erased from \a ent's model manager, not
  * from the underlying modeling kernel.
  *
  * Do *not* erase properties like name, color, and visibility unless
  * they are stored by the underlying modeling kernel in addition to
  * SMTK.
  *
  * Return true when at least one property was removed, false otherwise.
  */
bool Session::removeGeneratedProperties(const EntityRef& ent, SessionInfoBits propFlags)
{
  (void)ent;
  (void)propFlags;
  return false;
}

/// Subclasses implement this; it should add a record for \a entRef to the manager.
Entity* Session::addEntityRecord(const EntityRef& entRef)
{
  (void)entRef;
  return NULL;
}

/**\brief Subclasses implement this; it should return a new ArrangementHelper subclass instance.
  *
  * The caller is responsible for deleting it.
  */
ArrangementHelper* Session::createArrangementHelper()
{
  return new ArrangementHelper;
}

/**\brief Recursively called by transcribeInternal until no new entities are encountered.
  *
  */
int Session::findOrAddRelatedEntities(const EntityRef& entRef, SessionInfoBits flags, ArrangementHelper* helper)
{
  if (helper->isMarked(entRef))
    return 0;
  helper->mark(entRef, true);

  EntityTypeBits entType = static_cast<EntityTypeBits>(entRef.entityFlags() & ENTITY_MASK);
  // Ignore bits restricting group membership:
  if (entType & GROUP_ENTITY)
    entType = GROUP_ENTITY;

  int numAdded = 0;
  switch (entType)
    {
  case CELL_ENTITY:
    numAdded += this->findOrAddCellAdjacencies(entRef.as<CellEntity>(), flags, helper);
    numAdded += this->findOrAddCellUses(entRef.as<CellEntity>(), flags, helper);
    numAdded += this->findOrAddRelatedGroups(entRef, flags, helper);
    numAdded += this->findOrAddRelatedInstances(entRef, flags, helper);
    break;
  case USE_ENTITY:
    numAdded += this->findOrAddOwningCell(entRef.as<UseEntity>(), flags, helper);
    numAdded += this->findOrAddShellAdjacencies(entRef.as<UseEntity>(), flags, helper);
    numAdded += this->findOrAddRelatedGroups(entRef, flags, helper);
    numAdded += this->findOrAddRelatedInstances(entRef, flags, helper);
    break;
  case SHELL_ENTITY:
    numAdded += this->findOrAddUseAdjacencies(entRef.as<ShellEntity>(), flags, helper);
    numAdded += this->findOrAddRelatedGroups(entRef, flags, helper);
    numAdded += this->findOrAddRelatedInstances(entRef, flags, helper);
    break;
  case GROUP_ENTITY:
    numAdded += this->findOrAddGroupOwner(entRef.as<Group>(), flags, helper);
    numAdded += this->findOrAddRelatedGroups(entRef, flags, helper);
    numAdded += this->findOrAddRelatedInstances(entRef, flags, helper);
    break;
  case MODEL_ENTITY:
    numAdded += this->findOrAddFreeCells(entRef.as<Model>(), flags, helper);
    numAdded += this->findOrAddRelatedModels(entRef.as<Model>(), flags, helper);
    numAdded += this->findOrAddRelatedGroups(entRef, flags, helper);
    numAdded += this->findOrAddRelatedInstances(entRef, flags, helper);
    break;
  case INSTANCE_ENTITY:
    numAdded += this->findOrAddPrototype(entRef.as<Instance>(), flags, helper);
    numAdded += this->findOrAddRelatedGroups(entRef, flags, helper);
    break;
  case SESSION:
    numAdded += this->findOrAddRelatedModels(entRef.as<SessionRef>(), flags, helper);
    numAdded += this->findOrAddRelatedGroups(entRef, flags, helper);
    numAdded += this->findOrAddRelatedInstances(entRef, flags, helper);
    break;
  default:
    smtkInfoMacro(this->log(), "Unknown entity type " << entRef.entityFlags() << " being transcribed.");
    break;
    }

  helper->reset(entRef); // Remove all *generated* (not user-specified) arrangements, properties, etc.
  return numAdded;
}

/**\brief Subclasses implement this; it should add boundary, bounding, embedded, and embeddor cells of the current cell.
  *
  */
int Session::findOrAddCellAdjacencies(const CellEntity& entRef, SessionInfoBits request, ArrangementHelper* helper)
{
  (void)entRef;
  (void)request;
  (void)helper;
  return 0;
}

/**\brief Subclasses implement this; it should add use records of the current cell.
  *
  */
int Session::findOrAddCellUses(const CellEntity& entRef, SessionInfoBits request, ArrangementHelper* helper)
{
  (void)entRef;
  (void)request;
  (void)helper;
  return 0;
}

/**\brief Subclasses implement this; it should add the current use's owning cell.
  *
  */
int Session::findOrAddOwningCell(const UseEntity& entRef, SessionInfoBits request, ArrangementHelper* helper)
{
  (void)entRef;
  (void)request;
  (void)helper;
  return 0;
}

/**\brief Subclasses implement this; it should add shells bounded by or bounding the given use.
  *
  */
int Session::findOrAddShellAdjacencies(const UseEntity& entRef, SessionInfoBits request, ArrangementHelper* helper)
{
  (void)entRef;
  (void)request;
  (void)helper;
  return 0;
}

/**\brief
  *
  */
int Session::findOrAddUseAdjacencies(const ShellEntity& entRef, SessionInfoBits request, ArrangementHelper* helper)
{
  (void)entRef;
  (void)request;
  (void)helper;
  return 0;
}

/**\brief
  *
  */
int Session::findOrAddGroupOwner(const Group& entRef, SessionInfoBits request, ArrangementHelper* helper)
{
  (void)entRef;
  (void)request;
  (void)helper;
  return 0;
}

/**\brief
  *
  */
int Session::findOrAddFreeCells(const Model& entRef, SessionInfoBits request, ArrangementHelper* helper)
{
  (void)entRef;
  (void)request;
  (void)helper;
  return 0;
}

/**\brief
  *
  */
int Session::findOrAddRelatedModels(const Model& entRef, SessionInfoBits request, ArrangementHelper* helper)
{
  (void)entRef;
  (void)request;
  (void)helper;
  return 0;
}

/**\brief
  *
  */
int Session::findOrAddPrototype(const Instance& entRef, SessionInfoBits request, ArrangementHelper* helper)
{
  (void)entRef;
  (void)request;
  (void)helper;
  return 0;
}

/**\brief
  *
  */
int Session::findOrAddRelatedModels(const SessionRef& entRef, SessionInfoBits request, ArrangementHelper* helper)
{
  (void)entRef;
  (void)request;
  (void)helper;
  return 0;
}

/**\brief
  *
  */
int Session::findOrAddRelatedGroups(const EntityRef& entRef, SessionInfoBits request, ArrangementHelper* helper)
{
  (void)entRef;
  (void)request;
  (void)helper;
  return 0;
}

/**\brief
  *
  */
int Session::findOrAddRelatedInstances(const EntityRef& entRef, SessionInfoBits request, ArrangementHelper* helper)
{
  (void)entRef;
  (void)request;
  (void)helper;
  return 0;
}

/**\brief Subclasses implement this to finalize arrangement information for \a entRef using the \a helper.
  *
  */
SessionInfoBits Session::findOrAddArrangements(const EntityRef& entRef, Entity* entRec, SessionInfoBits flags, ArrangementHelper* helper)
{
  (void)entRef;
  (void)entRec;
  (void)flags;
  (void)helper;
  return SESSION_ARRANGEMENTS;
}

/**\brief Subclasses implement this to update transcribed (not user-specified) properties of \a entRef.
  *
  */
SessionInfoBits Session::updateProperties(const EntityRef& entRef, Entity* entRec, SessionInfoBits flags, ArrangementHelper* helper)
{
  (void)entRef;
  (void)entRec;
  (void)flags;
  (void)helper;
  return SESSION_FLOAT_PROPERTIES | SESSION_STRING_PROPERTIES | SESSION_INTEGER_PROPERTIES;
}

/**\brief Sublasses implement this to update the tessellation of the given \a entRef.
  *
  * This method will only be called when transcribe() is asked to include the tessellation.
  */
SessionInfoBits Session::updateTessellation(const EntityRef& entRef, SessionInfoBits flags, ArrangementHelper* helper)
{
  (void)entRef;
  (void)flags;
  (void)helper;
  return SESSION_TESSELLATION;
}

/**\brief Subclasses must call this method from within their constructors.
  *
  * Each subclass has (by virtue of invoking the smtkDeclareModelOperator
  * and smtkImplementsModelOperator macros) a static map from operator
  * names to constructors and XML descriptions. That map is named
  * s_operators and should be passed to this method in the constructor
  * of the subclass (since the base class does not have access to the map).
  *
  * This method traverses the XML descriptions and imports each into
  * the session's attribute system.
  */
void Session::initializeOperatorSystem(const OperatorConstructors* opList)
{
  // Superclasses may already have initialized, but since
  // we cannot remove Definitions from an attribute System
  // and may want to override an operator with a session-specific
  // version, we must wipe away whatever already exists.
  smtk::attribute::System* other = this->m_operatorSys;

  this->m_operatorSys = new smtk::attribute::System;
  // Create the "base" definitions that all operators and results will inherit.
  Definition::Ptr opdefn = this->m_operatorSys->createDefinition("operator");

  IntItemDefinition::Ptr assignNamesDefn = IntItemDefinition::New("assign names");
  // Do not assign names to entities after the operation by default:
  assignNamesDefn->setDefaultValue(0);
  assignNamesDefn->setIsOptional(true);
  assignNamesDefn->setAdvanceLevel(11);

  opdefn->addItemDefinition(assignNamesDefn);

  Definition::Ptr resultdefn = this->m_operatorSys->createDefinition("result");
  IntItemDefinition::Ptr outcomeDefn = IntItemDefinition::New("outcome");
  ModelEntityItemDefinition::Ptr entcreDefn = ModelEntityItemDefinition::New("created");
  ModelEntityItemDefinition::Ptr entmodDefn = ModelEntityItemDefinition::New("modified");
  ModelEntityItemDefinition::Ptr entremDefn = ModelEntityItemDefinition::New("expunged");

  StringItemDefinition::Ptr logDefn = StringItemDefinition::New("log");
  outcomeDefn->setNumberOfRequiredValues(1);
  outcomeDefn->setIsOptional(false);
  entcreDefn->setNumberOfRequiredValues(0);
  entcreDefn->setIsOptional(true);
  entcreDefn->setIsExtensible(true);
  entmodDefn->setNumberOfRequiredValues(0);
  entmodDefn->setIsOptional(true);
  entmodDefn->setIsExtensible(true);
  entremDefn->setNumberOfRequiredValues(0);
  entremDefn->setIsOptional(true);
  entremDefn->setIsExtensible(true);

  logDefn->setNumberOfRequiredValues(0);
  logDefn->setIsExtensible(1);
  logDefn->setIsOptional(true);

  resultdefn->addItemDefinition(outcomeDefn);
  resultdefn->addItemDefinition(entcreDefn);
  resultdefn->addItemDefinition(entmodDefn);
  resultdefn->addItemDefinition(entremDefn);
  resultdefn->addItemDefinition(logDefn);

  if (!opList && this->inheritsOperators())
    {
    delete this->m_operatorSys;
    this->m_operatorSys = other;
    return;
    }

  if (opList)
    {
    smtk::io::Logger tmpLog;
    smtk::io::AttributeReader rdr;
    OperatorConstructors::const_iterator it;
    bool ok = true;
    for (it = opList->begin(); it != opList->end(); ++it)
      {
      if (it->second.first.empty())
        continue;

      ok &= !rdr.readContents(
        *this->m_operatorSys,
        it->second.first.c_str(), it->second.first.size(),
        tmpLog);
      }
    if (!ok)
      {
      std::cerr
        << "Error. Log follows:\n---\n"
        << tmpLog.convertToString()
        << "\n---\n";
      }
    }

  if (other)
    {
    if (this->inheritsOperators())
      {
      // Copy definitions that do not already exist.
      std::vector<smtk::attribute::DefinitionPtr> tmp;
      std::vector<smtk::attribute::DefinitionPtr>::iterator it;

      DefinitionPtr otherOperator = other->findDefinition("operator");
      other->derivedDefinitions(otherOperator, tmp);
      for (it = tmp.begin(); it != tmp.end(); ++it)
        if (!this->m_operatorSys->findDefinition((*it)->type()))
          this->m_operatorSys->copyDefinition(*it);

      DefinitionPtr otherResult = other->findDefinition("result");
      other->derivedDefinitions(otherResult, tmp);
      for (it = tmp.begin(); it != tmp.end(); ++it)
        if (!this->m_operatorSys->findDefinition((*it)->type()))
          this->m_operatorSys->copyDefinition(*it);
      }

    delete other;
    }
}

/**\brief Import XML describing an operator into this session's operator system.
  *
  * This does not register a constructor for the operator;
  * it is meant for exposing operators registered after this session instance
  * has been constructed (and thus not defined by initializeOperatorSystem()),
  * so it should only be called from within registerOperator().
  */
void Session::importOperatorXML(const std::string& opXML)
{
  if (this->m_operatorSys && !opXML.empty())
    {
    smtk::io::AttributeReader rdr;
    bool ok = true;

    ok &= !rdr.readContents(
      *this->m_operatorSys,
      opXML.c_str(), opXML.size(),
      this->log());

    if (!ok)
      {
      std::cerr
        << "Error. Log follows:\n---\n"
        << this->log().convertToString()
        << "\n---\n";
      }
    }
}

/**\brief A convenience method used by subclass findOperatorXML methods.
  */
std::string Session::findOperatorXMLInternal(
  const std::string& opName,
  const OperatorConstructors* opList) const
{
  std::string xml;
  if (!opList)
    { // No operators registered.
    return xml;
    }
  smtk::model::OperatorConstructors::const_iterator it =
    opList->find(opName);
  if (it == opList->end())
    { // No matching operator.
    return xml;
    }
  return it->second.first;
}

#ifndef SHIBOKEN_SKIP
/**\brief A convenience method used by subclass findOperatorConstructor methods.
  */
OperatorConstructor Session::findOperatorConstructorInternal(
  const std::string& opName,
  const OperatorConstructors* opList) const
{
  if (!opList)
    { // No operators registered.
    return smtk::model::OperatorConstructor();
    }
  smtk::model::OperatorConstructors::const_iterator it =
    opList->find(opName);
  if (it == opList->end())
    { // No matching operator.
    return smtk::model::OperatorConstructor();
    }
  return it->second.second;
}
#endif // SHIBOKEN_SKIP

/**\brief Subclasses may override this method to export additional state.
  *
  * Importers (e.g., ImportJSON) and exporters (e.g., ExportJSON) will
  * call this method to obtain a session I/O class instance specific to
  * the given \a format. If a valid SessionIO shared-pointer is
  * returned, it will be dynamically cast to a format-specific subclass
  * and given the opportunity to provide additional information to be
  * imported/exported to/from the session.
  *
  * This default implementation is provided since most sessions will
  * not need additional state.
  */
SessionIOPtr Session::createIODelegate(const std::string& format)
{
  (void)format;
  return SessionIOPtr();
}

  } // namespace model
} // namespace smtk

smtkImplementsOperatorRegistration(
  smtk::model::Session,
  /* Do not inherit operators. */ false
);
