//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_remote_RemusConnection_h
#define __smtk_session_remote_RemusConnection_h

#include "smtk/bridge/remote/Exports.h"
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
  namespace io {
    class Logger;
  }
  namespace bridge {
    namespace remote {

class Session;

/**\brief Manage a connection to a Remus server used for modeling operations.
  *
  * Set this object up to talk to a specific Remus server
  * and the workers registered with the server.
  * As long as the worker is an smtk-remote-model
  * process (those should be the only ones that advertise
  * a service output type of "smtk[native]"), it will
  * respond to the JSON-RPC requests that instances of
  * this class issue.
  *
  * This class can manage multiple Session
  * objects that share a Remus server.
  * In the event that you do not provide a server
  * endpoint URL (i.e., you call connectToServer()
  * with the default parameters), this object will
  * create an in-process server.
  * In that case, you may also wish to call
  * addSearchDirectory().
  */
class SMTKREMOTESESSION_EXPORT RemusConnection : smtkEnableSharedPtr(RemusConnection)
{
public:
  smtkTypeMacro(RemusConnection);
  smtkCreateMacro(RemusConnection);
  virtual ~RemusConnection();

  void addSearchDir(const std::string& searchDir);
  void clearSearchDirs(bool clearDefaultsToo = false);
#ifndef SHIBOKEN_SKIP
  bool connectToServer(
    const std::string& hostname = "local",
    int port = remus::SERVER_CLIENT_PORT);
#else
  // Shiboken cannot parse the default port and does not
  // properly handle default arguments anyway, so provide
  // something for it to wrap:
  bool connectToServer(const std::string& hostname, int port);
#endif

  std::vector<std::string> sessionTypeNames();

  int staticSetup(
    const std::string& sessionName,
    const std::string& optName,
    const smtk::model::StringList& optVal);

#ifndef SHIBOKEN_SKIP
  smtk::common::UUID beginSession(const std::string& sessionName);
  bool endSession(const smtk::common::UUID& sessionId);
  SessionPtr findSession(
    const smtk::common::UUID& sessionId);

  smtk::model::StringData supportedFileTypes(
    const std::string& sessionName = std::string());
  smtk::model::OperatorResult readFile(
    const std::string& fileName,
    const std::string& fileType = std::string(),
    const std::string& sessionName = std::string());

  std::vector<std::string> operatorNames(const std::string& sessionName);
  std::vector<std::string> operatorNames(const smtk::common::UUID& sessionId);

  smtk::model::OperatorPtr createOperator(
    const smtk::common::UUID& sessionOrModel, const std::string& opName);
  smtk::model::OperatorPtr createOperator(
    const std::string& sessionName, const std::string& opName);

  void fetchWholeModel(const smtk::common::UUID& modelId);

  smtk::model::ManagerPtr modelManager();
  void setModelManager(smtk::model::ManagerPtr);

  cJSON* jsonRPCRequest(cJSON* req, const remus::proto::JobRequirements& jreq);
  cJSON* jsonRPCRequest(const std::string& req, const remus::proto::JobRequirements& jreq);
  void jsonRPCNotification(cJSON* req, const remus::proto::JobRequirements& jreq);
  void jsonRPCNotification(const std::string& req, const remus::proto::JobRequirements& jreq);

  remus::client::ServerConnection connection();

  smtk::io::Logger& log();

protected:
  RemusConnection();

  smtk::shared_ptr<Session> findSessionForRemusType(const std::string& rtype);
  bool findRequirementsForRemusType(remus::proto::JobRequirements& jreq, const std::string& rtype);
  std::string createNameFromTags(cJSON* tags);

  typedef std::set<std::string> searchdir_t;
  searchdir_t m_searchDirs;
  remus::client::ServerConnection m_conn;
  smtk::shared_ptr<remus::client::Client> m_client;
  smtk::shared_ptr<remus::Server> m_localServer;
  smtk::model::ManagerPtr m_modelMgr;
  std::map<std::string,std::string> m_remoteSessionNameToType;
  std::map<smtk::common::UUID,std::string> m_remoteSessionRefIds;
#endif // SHIBOKEN_SKIP
};

    } // namespace remote
  } // namespace bridge
} // namespace smtk

#endif // __smtk_session_remote_RemusConnection_h
