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
  const EntityRef& entity, SessionInfoBits requested, bool onlyDangling)
{
  int retval = 0;
  if (requested)
    {
    // Check that the entity IDs is dangling or we are forced to continue.
    DanglingEntities::iterator it = this->m_dangling.find(entity);
    if (onlyDangling && it == this->m_dangling.end())
      { // The session has not been told that this UUID exists.
      return retval;
      }
    // Ask the subclass to transcribe information.
    SessionInfoBits actual = this->transcribeInternal(entity, requested);
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
StringList Session::operatorNames() const
{
  std::vector<smtk::attribute::DefinitionPtr> ops;
  this->m_operatorSys->derivedDefinitions(
    this->m_operatorSys->findDefinition("operator"), ops);

  StringList nameList;
  std::vector<smtk::attribute::DefinitionPtr>::iterator it;
  for (it = ops.begin(); it != ops.end(); ++it)
    nameList.push_back((*it)->type());
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

/**\brief Transcribe information requested by \a flags into \a entity from foreign modeler.
  *
  * Subclasses must override this method.
  * This method should return a non-zero value upon success.
  * Upon success, \a flags should be modified to represent the
  * actual information transcribed (as opposed to what was requested).
  * This should always be at least the information requested but may
  * include more information.
  */
SessionInfoBits Session::transcribeInternal(const EntityRef& entity, SessionInfoBits flags)
{
  (void)entity;
  (void)flags;
  // Fail to transcribe anything:
  return 0;
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
  ModelEntityItemDefinition::Ptr entoutDefn = ModelEntityItemDefinition::New("entities");
  ModelEntityItemDefinition::Ptr entremDefn = ModelEntityItemDefinition::New("expunged");

  StringItemDefinition::Ptr logDefn = StringItemDefinition::New("log");
  outcomeDefn->setNumberOfRequiredValues(1);
  outcomeDefn->setIsOptional(false);
  entoutDefn->setNumberOfRequiredValues(0);
  entoutDefn->setIsOptional(true);
  entoutDefn->setIsExtensible(true);
  entremDefn->setNumberOfRequiredValues(0);
  entremDefn->setIsOptional(true);
  entremDefn->setIsExtensible(true);

  logDefn->setNumberOfRequiredValues(0);
  logDefn->setIsExtensible(1);
  logDefn->setIsOptional(true);

  resultdefn->addItemDefinition(outcomeDefn);
  resultdefn->addItemDefinition(entoutDefn);
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
    smtk::io::Logger log;
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
        log);
      }
    if (!ok)
      {
      std::cerr
        << "Error. Log follows:\n---\n"
        << log.convertToString()
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
    smtk::io::Logger log;
    smtk::io::AttributeReader rdr;
    bool ok = true;

    ok &= !rdr.readContents(
      *this->m_operatorSys,
      opXML.c_str(), opXML.size(),
      log);

    if (!ok)
      {
      std::cerr
        << "Error. Log follows:\n---\n"
        << log.convertToString()
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
