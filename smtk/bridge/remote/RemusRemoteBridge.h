//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_bridge_remote_RemusRemoteBridge_h
#define __smtk_bridge_remote_RemusRemoteBridge_h

#ifndef SHIBOKEN_SKIP
#  include "smtk/model/BridgeRegistrar.h"
#  include "smtk/bridge/remote/RemusStaticBridgeInfo.h"
#endif // SHIBOKEN_SKIP
#include "smtk/bridge/remote/SMTKRemoteExports.h" // for export macro
#include "smtk/SharedPtr.h" // for export macro
#include "smtk/model/DefaultBridge.h"
#include "smtk/model/StringData.h"

#ifndef _MSC_VER
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
#endif
#include "remus/client/Client.h" // for m_remusClient
#include "remus/worker/ServerConnection.h" // for m_remusConn
#ifndef _MSC_VER
  #pragma GCC diagnostic pop
#endif

namespace smtk {
  namespace io { class ImportJSON; }
  namespace model { class RemoteOperator; }
  namespace bridge {
    namespace remote {

class RemusBridgeConnection; // A Remus client-server connection specifically for smtk::models.

/**\brief A bridge that forwards operation requests to a Remus worker.
  *
  * This bridge does no transcription because
  * it forwards all requests to a remote bridge (which is
  * backed with local storage).
  *
  * Instances of this bridge report their name() as "remote".
  * Its remoteName() will be the type of the bridge they provide
  * backing storage for.
  *
  * SMTK does not provide transport for forwarding requests;
  * instead it uses Remus for this.
  * For a protocol, it uses JSON-RPC v2.
  */
class SMTKREMOTE_EXPORT RemusRemoteBridge : public smtk::model::DefaultBridge
{
public:
  smtkTypeMacro(RemusRemoteBridge);
  smtkCreateMacro(RemusRemoteBridge);
  smtkSharedFromThisMacro(Bridge);
  smtkDeclareModelingKernel();

  virtual ~RemusRemoteBridge();

  Ptr setup(RemusBridgeConnection* remusServerConnection, remus::proto::JobRequirements& jreq);
  remus::proto::JobRequirements remusRequirements() const;

protected:
  friend class model::RemoteOperator;
  friend class io::ImportJSON;
  friend class RemusBridgeConnection;

  RemusRemoteBridge();

  virtual smtk::model::BridgedInfoBits transcribeInternal(
    const smtk::model::Cursor& entity, smtk::model::BridgedInfoBits flags);

  virtual bool ableToOperateDelegate(smtk::model::RemoteOperatorPtr op);
  virtual smtk::model::OperatorResult operateDelegate(
    smtk::model::RemoteOperatorPtr op);

#ifndef SHIBOKEN_SKIP
  static RemusStaticBridgeInfo createFunctor(
    RemusBridgeConnectionPtr remusConn,
    const remus::proto::JobRequirements& jobReq,
    const std::string& meshType);
#endif // SHIBOKEN_SKIP
  static bool registerBridgedOperator(
    const std::string& bridgeName, const std::string& opName,
    const char* opDescrXML, smtk::model::OperatorConstructor opCtor);

  RemusBridgeConnection* m_remusConn;
  smtk::shared_ptr<remus::client::Client> m_remusClient;
  std::string m_remusWorkerName;
  remus::proto::JobRequirements m_remusWorkerReqs;

  static std::map<std::string,RemusStaticBridgeInfo>* s_remotes;

  static void cleanupBridgeTypes();
};

    } // namespace remote
  } // namespace bridge
} // namespace smtk

#endif // __smtk_bridge_remote_RemusRemoteBridge_h
