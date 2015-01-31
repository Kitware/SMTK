//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_remote_Session_h
#define __smtk_session_remote_Session_h

#ifndef SHIBOKEN_SKIP
#  include "smtk/model/SessionRegistrar.h"
#  include "smtk/bridge/remote/RemusStaticSessionInfo.h"
#endif // SHIBOKEN_SKIP
#include "smtk/bridge/remote/SMTKRemoteExports.h" // for export macro
#include "smtk/SharedPtr.h" // for export macro
#include "smtk/model/DefaultSession.h"
#include "smtk/model/StringData.h"

#ifndef SHIBOKEN_SKIP
#ifndef _MSC_VER
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
#endif
#include "remus/client/Client.h" // for m_remusClient
#include "remus/worker/ServerConnection.h" // for m_remusConn
#ifndef _MSC_VER
  #pragma GCC diagnostic pop
#endif
#endif // SHIBOKEN_SKIP

namespace smtk {
  namespace io { class ImportJSON; }
  namespace model { class RemoteOperator; }
  namespace bridge {
    namespace remote {

class RemusConnection; // A Remus client-server connection specifically for smtk::models.

/**\brief A session that forwards operation requests to a Remus worker.
  *
  * This session does no transcription because
  * it forwards all requests to a remote session (which is
  * backed with local storage).
  *
  * Instances of this session report their name() as "remote".
  * Its remoteName() will be the type of the session they provide
  * backing storage for.
  *
  * SMTK does not provide transport for forwarding requests;
  * instead it uses Remus for this.
  * For a protocol, it uses JSON-RPC v2.
  */
class SMTKREMOTE_EXPORT Session : public smtk::model::DefaultSession
{
public:
  smtkTypeMacro(Session);
  smtkCreateMacro(smtk::model::Session);
  smtkSharedFromThisMacro(smtk::model::Session);
  smtkSuperclassMacro(smtk::model::DefaultSession);
  smtkDeclareModelingKernel();

  virtual ~Session();

  using smtk::model::Session::setup;
#ifndef SHIBOKEN_SKIP
  Ptr setup(RemusConnection* remusServerConnection, remus::proto::JobRequirements& jreq);
  remus::proto::JobRequirements remusRequirements() const;

protected:
  friend class model::RemoteOperator;
  friend class io::ImportJSON;
  friend class RemusConnection;

  Session();

  virtual smtk::model::SessionInfoBits transcribeInternal(
    const smtk::model::EntityRef& entity, smtk::model::SessionInfoBits flags);

  virtual bool ableToOperateDelegate(smtk::model::RemoteOperatorPtr op);
  virtual smtk::model::OperatorResult operateDelegate(
    smtk::model::RemoteOperatorPtr op);

  static RemusStaticSessionInfo createFunctor(
    RemusConnectionPtr remusConn,
    const remus::proto::JobRequirements& jobReq,
    const std::string& meshType);
  static bool registerSessionOperator(
    const std::string& sessionName, const std::string& opName,
    const char* opDescrXML, smtk::model::OperatorConstructor opCtor);

  RemusConnection* m_remusConn;
  smtk::shared_ptr<remus::client::Client> m_remusClient;
  std::string m_remusWorkerName;
  remus::proto::JobRequirements m_remusWorkerReqs;

  static std::map<std::string,RemusStaticSessionInfo>* s_remotes;

  static void cleanupSessionTypes();
#endif // SHIBOKEN_SKIP
};

    } // namespace remote
  } // namespace bridge
} // namespace smtk

#endif // __smtk_session_remote_Session_h
