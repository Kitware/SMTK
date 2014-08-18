#ifndef __smtk_remote_RemusBridgeConnection_h
#define __smtk_remote_RemusBridgeConnection_h

#include "smtk/SMTKRemoteExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/util/SharedFromThis.h"
#include "smtk/util/UUID.h"

#include "remus/client/Client.h"
#include "remus/client/ServerConnection.h"

#include "remus/proto/JobRequirements.h"

#include "cJSON.h"

#include <map>
#include <set>
#include <string>

namespace smtk {
  namespace model {

class RemusRemoteBridge;

/**\brief Provide JSON-RPC over a Remus connection.
  *
  * Set this object up to talk to a specific remus server
  * and worker (registered with the server).
  * As long as the worker is an smtk-remote-model
  * process, it will respond to the JSON-RPC requests
  * instances of this class issue.
  */
class SMTKREMOTE_EXPORT RemusBridgeConnection
{
public:
  smtkTypeMacro(RemusBridgeConnection);
  smtkCreateMacro(RemusBridgeConnection);
  virtual ~RemusBridgeConnection();

  bool connectToServer(const std::string& hostname, int port);

  std::vector<std::string> bridgeNames();

  smtk::util::UUID beginBridgeSession(const std::string& bridgeName);
  bool endBridgeSession(const smtk::util::UUID& bridgeSessionId);

  std::vector<std::string> supportedFileTypes(
    const std::string& bridgeName = std::string());
  smtk::model::OperatorResult readFile(
    const std::string& fileName,
    const std::string& fileType = std::string(),
    const std::string& bridgeName = std::string());

  std::vector<std::string> operatorNames(const std::string& bridgeName);
  std::vector<std::string> operatorNames(const smtk::util::UUID& bridgeSessionId);

  smtk::model::OperatorPtr createOperator(
    const smtk::util::UUID& bridgeOrModel, const std::string& opName);
  smtk::model::OperatorPtr createOperator(
    const std::string& bridgeName, const std::string& opName);

  void fetchWholeModel(const smtk::util::UUID& modelId);

  smtk::model::ManagerPtr modelManager();
  void setModelManager(smtk::model::ManagerPtr);

  cJSON* jsonRPCRequest(cJSON* req, const remus::proto::JobRequirements& jreq);
  cJSON* jsonRPCRequest(const std::string& req, const remus::proto::JobRequirements& jreq);
  void jsonRPCNotification(cJSON* req, const remus::proto::JobRequirements& jreq);
  void jsonRPCNotification(const std::string& req, const remus::proto::JobRequirements& jreq);

protected:
  RemusBridgeConnection();

  smtk::shared_ptr<RemusRemoteBridge> findBridgeForRemusType(const std::string& rtype);
  bool findRequirementsForRemusType(remus::proto::JobRequirements& jreq, const std::string& rtype);

  remus::client::ServerConnection m_conn;
  smtk::shared_ptr<remus::client::Client> m_client;
  smtk::model::ManagerPtr m_modelMgr;
  std::set<std::string> m_remoteBridgeNames;
  std::map<smtk::util::UUID,std::string> m_remoteBridgeSessionIds;
};

  } // namespace model
} // namespace smtk

#endif // __smtk_remote_RemusBridgeConnection_h
