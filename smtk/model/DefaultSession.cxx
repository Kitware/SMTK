//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/DefaultSession.h"

#include "smtk/model/SessionRegistrar.h"
#include "smtk/model/BRepModel.h"
#include "smtk/model/RemoteOperator.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"

#include "smtk/AutoInit.h"

namespace smtk {
  namespace model {

/// Default constructor. Initializes statically-registered operators.
DefaultSession::DefaultSession()
{
  this->initializeOperatorSystem(DefaultSession::s_operators);
}

/// Indicate that, since we have no "backing store" model, the entire model is already present.
SessiondInfoBits DefaultSession::transcribeInternal(const EntityRef& entity, SessiondInfoBits flags)
{
  (void)entity;
  (void)flags;
  return SESSION_EVERYTHING;
}

/**\brief Call this method to indicate that the session acts
  *       as a backing store for a remote session.
  *
  * Some applications built on SMTK need to present information about
  * a model on a client that is remote to the geometric modeling kernel.
  * This call indicates that the session should be used to locally mirror
  * operations on the server.
  *
  * All this does is set the session's name and session ID to match
  * the remote session.
  * Subclasses are responsible for overriding transcribeInternal() to
  * fetch records from the remote session's model manager on demand.
  */
void DefaultSession::backsRemoteSession(
  const std::string& remoteSessionName,
  const smtk::common::UUID& sessionSessionId)
{
  this->m_remoteSessionName = remoteSessionName;
  this->m_sessionId = sessionSessionId;
}

/**\brief Returns an empty string or, when backsRemoteSession
  *       has been called, the type-name of the remote session.
  *
  */
std::string DefaultSession::remoteName() const
{
  return this->m_remoteSessionName;
}

/**\brief Return an instance of the operator of the given name, if it exists.
  *
  * Under some circumstances, a RemoteOperator will be created and returned:
  * 1. No existing operator matches the given name.
  * 2. The remoteName() method returns a non-empty string (i..e, backsRemoteSession
  *    has been called).
  * 3. A friend class (such as ImportJSON) has called setImportingOperators(true)
  *    and not subsequently called setImportingOperators(false).
  */
OperatorPtr DefaultSession::op(const std::string& opName) const
{
  OperatorPtr oper = this->Session::op(opName);
  if (!oper && !this->m_remoteSessionName.empty())
    { // we are a remote session... create any operator our friend classes ask for.
    RemoteOperatorPtr rop = RemoteOperator::create();
    rop->setName(opName);
    rop->setManager(this->manager());
    // Naughty, but necessary so we can pretend that the
    // operator existed all along.
    DefaultSession* self = const_cast<DefaultSession*>(this);
    rop->setSession(self);
    oper = rop;
    }
  return oper;
}

/**@name RemoteOperator delegate methods.
  *
  * If this session has any operators derived from RemoteOperator,
  * those operators will call these methods when their ableToOperate()
  * or operate() members are invoked.
  * It is the session's responsibility to forward
  * the requests to the appropriate, non-virtual session for
  * execution and return the results.
  */
///@{
/**\brief A delegate for the RemoteOperator::ableToOperate() method.
  *
  * The DefaultSession implementation does nothing.
  * Subclasses must override this method.
  */
bool DefaultSession::ableToOperateDelegate(RemoteOperatorPtr oper)
{
  (void)oper;
  return false;
}

/**\brief A delegate for the RemoteOperator::operate() method.
  *
  * The DefaultSession implementation does nothing.
  * Subclasses must override this method.
  */
OperatorResult DefaultSession::operateDelegate(RemoteOperatorPtr oper)
{
  if (!oper)
    return OperatorResult();

  return oper->createResult(OPERATION_FAILED);
}
///@}

  } // namespace model
} // namespace smtk

#include "smtk/model/DefaultSession_json.h" // For DefaultSession_json
smtkImplementsModelingKernel(
  native,
  DefaultSession_json,
  smtk::model::SessionHasNoStaticSetup,
  smtk::model::DefaultSession,
  true /* inherit "universal" operators */
);
