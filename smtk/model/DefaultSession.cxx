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

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"

#include "smtk/AutoInit.h"

namespace smtk
{
namespace model
{

/// Default constructor. Initializes statically-registered operators.
DefaultSession::DefaultSession() = default;

/// Indicate that, since we have no "backing store" model, the entire model is already present.
SessionInfoBits DefaultSession::transcribeInternal(
  const EntityRef& entity, SessionInfoBits flags, int depth)
{
  (void)entity;
  (void)flags;
  (void)depth;
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
  * fetch records from the remote session's model resource on demand.
  */
void DefaultSession::backsRemoteSession(
  const std::string& remoteSessionName, const smtk::common::UUID& sessId)
{
  m_remoteSessionName = remoteSessionName;
  m_sessionId = sessId;
}

/**\brief Returns an empty string or, when backsRemoteSession
  *       has been called, the type-name of the remote session.
  *
  */
std::string DefaultSession::remoteName() const
{
  return m_remoteSessionName;
}

/**\brief Return an instance of the operator of the given name, if it exists.
  *
  * Under some circumstances, a RemoteOperation will be created and returned:
  * 1. No existing operator matches the given name.
  * 2. The remoteName() method returns a non-empty string (i..e, backsRemoteSession
  *    has been called).
  * 3. A friend class (such as LoadJSON) has called setImportingOperations(true)
  *    and not subsequently called setImportingOperations(false).
  */
// OperationPtr DefaultSession::op(const std::string& opName) const
// {
//   OperationPtr oper = this->Session::op(opName);
//   return oper;
// }

} // namespace model
} // namespace smtk
