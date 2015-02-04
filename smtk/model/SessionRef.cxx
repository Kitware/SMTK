//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/SessionRef.h"

#include "smtk/attribute/Definition.h"
#include "smtk/attribute/System.h"

#include "smtk/model/SessionRegistrar.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/Model.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Arrangement.h"

namespace smtk {
  namespace model {

/**\brief Alternative constructor using session pointer instead of ID.
  *
  * This constructor obtains the session ID from the
  * \a brdg object.
  *
  * This variant also ensures that the session is registered
  * with the \a manager.
  */
SessionRef::SessionRef(ManagerPtr mgr, SessionPtr brdg)
  : EntityRef(mgr, brdg->sessionId())
{
  if (!!mgr && !mgr->sessionData(*this))
    mgr->registerSession(brdg);
}

/**\brief Return the actual session this entityref references (or null).
  *
  */
Session::Ptr SessionRef::session() const
{
  ManagerPtr mgr = this->m_manager.lock();
  if (!mgr || !this->m_entity)
    return Session::Ptr();
  return mgr->sessionData(*this);
}

/**\brief Return the list of operations this session supports.
  *
  */
StringList SessionRef::operatorNames() const
{
  Session::Ptr brdg = this->session();
  if (!brdg)
    return StringList();
  return brdg->operatorNames();
}

/**\brief Return the smtk::attribute::System holding all the operator definitions.
  *
  */
smtk::attribute::System* SessionRef::opSys() const
{
  Session::Ptr brdg = this->session();
  if (!brdg)
    return NULL;
  return brdg->operatorSystem();
}

/**\brief Return the smtk::attribute::Definition describing an operator.
  *
  */
OperatorDefinition SessionRef::opDef(const std::string& opName) const
{
  smtk::attribute::System* sys = this->opSys();
  if (!sys)
    return OperatorDefinition();
  return sys->findDefinition(opName);
}

/**\brief Return an instance of an operator given its name.
  *
  */
Operator::Ptr SessionRef::op(const std::string& opName) const
{
  Session::Ptr brdg = this->session();
  if (!brdg)
    return Operator::Ptr();
  return brdg->op(opName);
}

/**\brief Return operators that can be associated with the given entity flags.
  *
  */
StringList SessionRef::operatorsForAssociation(BitFlags assocMask) const
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

/**\brief Return the session-class's tag data (a JSON string).
  *
  */
std::string SessionRef::tag() const
{
  return SessionRegistrar::sessionTags(this->session()->name());
}

/**\brief Return the session-class's site name.
  *
  * This will be empty for local sessions.
  */
std::string SessionRef::site() const
{
  return SessionRegistrar::sessionSite(this->session()->name());
}

/**\brief Return the session-class's list of engines.
  *
  * Engines are different modeling backends that can be used.
  * Often, an instance of a session can only support a single engine
  * even if support for several engines is available.
  */
StringList SessionRef::engines() const
{
  return SessionRegistrar::sessionEngines(this->session()->name());
}

/**\brief Return the list of file types supported by this session.
  *
  * Passing in an engine name and context will return file types
  * specific to that \a engine in that \a context.
  *
  * Valid context strings are "read", "import", "write", and "export".
  * The default is "read".
  */
StringData SessionRef::fileTypes(
  const std::string& engine) const
{
  return SessionRegistrar::sessionFileTypes(this->session()->name(), engine);
}

void SessionRef::close()
{
  ManagerPtr mgr = this->manager();
  if (mgr)
    mgr->closeSession(*this);
}

/*! \fn template<typename T> T SessionRef::models() const;
  * \brief Return the list of models associated with this session.
  *
  * This returns all of the models for which Manager::setSessionForModel()
  * has been called with this entityref's session.
  */

/*! \fn template<typename T> StringList SessionRef::operatorsForAssociationconst T& entityrefContainer) const
  * \brief Return operators that can be associated with the given entity flags.
  *
  * This list is obtained by bitwise-ANDing all of the entity flags of
  * the entityrefs in the given \a entityrefContainer and calling
  * another variant of operatorsForAssociation with the resulting mask.
  * It exits early if the mask is empty.
  *
  * TODO: Handle recursive testing of groups like that done
  *       by GroupEntitity::meetsMembershipConstraints.
  */

  } // namespace model
} // namespace smtk
