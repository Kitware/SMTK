//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_bridge_remote_RemusBridgeConnection_h
#define __smtk_bridge_remote_RemusBridgeConnection_h

#include "smtk/bridge/remote/SMTKRemoteExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"

#include "smtk/model/StringData.h"

#include "smtk/common/UUID.h"

#ifndef SHIBOKEN_SKIP
#include "remus/client/Client.h"
#include "remus/client/ServerConnection.h"

#include "remus/server/Server.h"

#include "remus/common/remusGlobals.h"

#include "remus/proto/JobRequirements.h"

#include "cJSON.h"
#endif // SHIBOKEN_SKIP

#include <map>
#include <set>
#include <string>

namespace smtk {
  namespace bridge {
    namespace remote {

class RemusRemoteBridge;

/**\brief Provide JSON-RPC over a Remus connection.
  *
  * Set this object up to talk to a specific remus server
  * and worker (registered with the server).
  * As long as the worker is an smtk-remote-model
  * process, it will respond to the JSON-RPC requests
  * instances of this class issue.
  *
  * This class can manage multiple RemusRemoteBridge
  * objects that share a Remus server.
  * In the event that you do not provide a server
  * endpoint URL (i.e., you call connectToServer()
  * with the default parameters), this object will
  * create an in-process server.
  * In that case, you may also wish to call
  * addSearchDirectory().
  */
class SMTKREMOTE_EXPORT RemusBridgeConnection : smtkEnableSharedPtr(RemusBridgeConnection)
{
public:
  smtkTypeMacro(RemusBridgeConnection);
  smtkCreateMacro(RemusBridgeConnection);
  virtual ~RemusBridgeConnection();

  void addSearchDir(const std::string& searchDir);
  void clearSearchDirs(bool clearDefaultsToo = false);
#ifndef SHIBOKEN_SKIP
  bool connectToServer(
    const std::string& hostname = "local",
    int port = remus::SERVER_CLIENT_PORT);

  std::vector<std::string> bridgeNames();

  int staticSetup(
    const std::string& bridgeName,
    const std::string& optName,
    const smtk::model::StringList& optVal);

  smtk::common::UUID beginBridgeSession(const std::string& bridgeName);
  bool endBridgeSession(const smtk::common::UUID& bridgeSessionId);
  RemusRemoteBridgePtr findBridgeSession(
    const smtk::common::UUID& bridgeSessionId);

  std::vector<std::string> supportedFileTypes(
    const std::string& bridgeName = std::string());
  smtk::model::OperatorResult readFile(
    const std::string& fileName,
    const std::string& fileType = std::string(),
    const std::string& bridgeName = std::string());

  std::vector<std::string> operatorNames(const std::string& bridgeName);
  std::vector<std::string> operatorNames(const smtk::common::UUID& bridgeSessionId);

  smtk::model::OperatorPtr createOperator(
    const smtk::common::UUID& bridgeOrModel, const std::string& opName);
  smtk::model::OperatorPtr createOperator(
    const std::string& bridgeName, const std::string& opName);

  void fetchWholeModel(const smtk::common::UUID& modelId);

  smtk::model::ManagerPtr modelManager();
  void setModelManager(smtk::model::ManagerPtr);

  cJSON* jsonRPCRequest(cJSON* req, const remus::proto::JobRequirements& jreq);
  cJSON* jsonRPCRequest(const std::string& req, const remus::proto::JobRequirements& jreq);
  void jsonRPCNotification(cJSON* req, const remus::proto::JobRequirements& jreq);
  void jsonRPCNotification(const std::string& req, const remus::proto::JobRequirements& jreq);

  remus::client::ServerConnection connection();

protected:
  RemusBridgeConnection();

  smtk::shared_ptr<RemusRemoteBridge> findBridgeForRemusType(const std::string& rtype);
  bool findRequirementsForRemusType(remus::proto::JobRequirements& jreq, const std::string& rtype);
  std::string createNameFromTags(cJSON* tags);

  typedef std::set<std::string> searchdir_t;
  searchdir_t m_searchDirs;
  remus::client::ServerConnection m_conn;
  smtk::shared_ptr<remus::client::Client> m_client;
  smtk::shared_ptr<remus::Server> m_localServer;
  smtk::model::ManagerPtr m_modelMgr;
  std::set<std::string> m_remoteBridgeNames;
  std::map<smtk::common::UUID,std::string> m_remoteBridgeSessionIds;
#endif // SHIBOKEN_SKIP
};

    } // namespace remote
  } // namespace bridge
} // namespace smtk

#endif // __smtk_bridge_remote_RemusBridgeConnection_h
