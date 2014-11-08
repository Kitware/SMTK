//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/BridgeSession.h"

#include "smtk/attribute/Definition.h"
#include "smtk/attribute/System.h"

#include "smtk/model/BridgeRegistrar.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Arrangement.h"

namespace smtk {
  namespace model {

/**\brief Alternative constructor using bridge pointer instead of ID.
  *
  * This constructor obtains the bridge session ID from the
  * \a brdg object.
  *
  * This variant also ensures that the bridge is registered
  * with the \a manager.
  */
BridgeSession::BridgeSession(ManagerPtr mgr, BridgePtr brdg)
  : Cursor(mgr, brdg->sessionId())
{
  if (!this->m_manager->findBridgeSession(brdg->sessionId()))
    this->m_manager->registerBridgeSession(brdg);
}

/**\brief Return the actual bridge this cursor references (or null).
  *
  */
Bridge::Ptr BridgeSession::bridge() const
{
  if (!this->m_manager || !this->m_entity)
    return Bridge::Ptr();
  return this->m_manager->findBridgeSession(this->m_entity);
}

/**\brief Return the list of operations this bridge supports.
  *
  */
StringList BridgeSession::operatorNames() const
{
  Bridge::Ptr brdg = this->bridge();
  if (!brdg)
    return StringList();
  return brdg->operatorNames();
}

/**\brief Return the smtk::attribute::System holding all the operator definitions.
  *
  */
smtk::attribute::System* BridgeSession::opSys() const
{
  Bridge::Ptr brdg = this->bridge();
  if (!brdg)
    return NULL;
  return brdg->operatorSystem();
}

/**\brief Return the smtk::attribute::Definition describing an operator.
  *
  */
OperatorDefinition BridgeSession::opDef(const std::string& opName) const
{
  smtk::attribute::System* sys = this->opSys();
  if (!sys)
    return OperatorDefinition();
  return sys->findDefinition(opName);
}

/**\brief Return an instance of an operator given its name.
  *
  */
Operator::Ptr BridgeSession::op(const std::string& opName) const
{
  Bridge::Ptr brdg = this->bridge();
  if (!brdg)
    return Operator::Ptr();
  return brdg->op(opName, this->m_manager);
}

/**\brief Return operators that can be associated with the given entity flags.
  *
  */
StringList BridgeSession::operatorsForAssociation(BitFlags assocMask) const
{
  StringList result;
  smtk::attribute::System* sys = this->opSys();
  if (!assocMask || !sys)
    return result;

  std::vector<smtk::attribute::DefinitionPtr> defs;
  std::vector<smtk::attribute::DefinitionPtr>::iterator it;
  sys->findDefinitions(assocMask, defs);
  for (it = defs.begin(); it != defs.end(); ++it)
    result.push_back((*it)->type());

  return result;
}

/**\brief Return the bridge-class's tag data (a JSON string).
  *
  */
std::string BridgeSession::tag() const
{
  return BridgeRegistrar::bridgeTags(this->bridge()->name());
}

/**\brief Return the bridge-class's site name.
  *
  * This will be empty for local bridges.
  */
std::string BridgeSession::site() const
{
  return BridgeRegistrar::bridgeSite(this->bridge()->name());
}

/**\brief Return the bridge-class's list of engines.
  *
  * Engines are different modeling backends that can be used.
  * Often, an instance of a bridge can only support a single engine
  * even if support for several engines is available.
  */
StringList BridgeSession::engines() const
{
  return BridgeRegistrar::bridgeEngines(this->bridge()->name());
}

/**\brief Return the list of file types supported by this bridge.
  *
  * Passing in an engine name and context will return file types
  * specific to that \a engine in that \a context.
  *
  * Valid context strings are "read", "import", "write", and "export".
  * The default is "read".
  */
StringData BridgeSession::fileTypes(
  const std::string& engine) const
{
  return BridgeRegistrar::bridgeFileTypes(this->bridge()->name(), engine);
}

/*! \fn template<typename T> T BridgeSession::models() const;
  * \brief Return the list of models associated with this bridge.
  *
  * This returns all of the models for which BRepModel::setBridgeForModel()
  * has been called with this cursor's bridge.
  */

/*! \fn template<typename T> StringList BridgeSession::operatorsForAssociationconst T& cursorContainer) const
  * \brief Return operators that can be associated with the given entity flags.
  *
  * This list is obtained by bitwise-ANDing all of the entity flags of
  * the cursors in the given \a cursorContainer and calling
  * another variant of operatorsForAssociation with the resulting mask.
  * It exits early if the mask is empty.
  *
  * TODO: Handle recursive testing of groups like that done
  *       by GroupEntitity::meetsMembershipConstraints.
  */

  } // namespace model
} // namespace smtk
