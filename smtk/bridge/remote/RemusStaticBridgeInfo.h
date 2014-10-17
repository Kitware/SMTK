#ifndef __smtk_bridge_remote_RemusStaticBridgeInfo_h
#define __smtk_bridge_remote_RemusStaticBridgeInfo_h
#ifndef SHIBOKEN_SKIP

#include "smtk/PublicPointerDefs.h"
#include "smtk/bridge/remote/SMTKRemoteExports.h"

#include "smtk/model/StringData.h"

#include <string>

namespace remus {
  namespace proto {
    class JobRequirements;
  }
}

namespace smtk {
  namespace bridge {
    namespace remote {

/**\brief A helper class for the Remus remote bridge.
  *
  * Instances of this class can be passed as a BridgeConstructor,
  * which is an smtk::function<BridgePtr()>.
  * In addition to simply creating a new bridge, they
  * hold metadata about how the bridge should be configured
  * and can invoke methods to prepare the bridge for use with
  * a specific Remus worker that has been registered.
  */
class SMTKREMOTE_EXPORT RemusStaticBridgeInfo
{
public:
  RemusStaticBridgeInfo();
  RemusStaticBridgeInfo(
    RemusBridgeConnectionPtr conn,
    const remus::proto::JobRequirements& jobReq,
    const std::string& meshType
  );
  RemusStaticBridgeInfo(
    const RemusStaticBridgeInfo&);

  int staticSetup(
    const std::string& optName,
    const smtk::model::StringList& optVal);

  smtk::model::BridgePtr operator () () const;

  std::string name() const { return this->m_name; }
  std::string tags() const { return this->m_tags; }

  RemusBridgeConnectionPtr m_conn;
  std::string m_meshType;
  std::string m_name;
  std::string m_tags;
  std::string m_operatorXML;
};

    } // namespace remote
  } // namespace bridge
} // namespace smtk

#endif // SHIBOKEN_SKIP
#endif // __smtk_bridge_remote_RemusStaticBridgeInfo_h
