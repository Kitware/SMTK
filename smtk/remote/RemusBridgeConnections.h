#ifndef __smtk_remote_RemusBridgeConnections_h
#define __smtk_remote_RemusBridgeConnections_h

#include "smtk/SMTKRemoteExports.h"
#include "smtk/util/SharedFromThis.h"

#include <string>

namespace smtk {
  namespace model {

class RemusBridgeConnection;

/**\brief Maintain a list of model-server connections.
  *
  * Applications should keep a single instance of this class
  * and add connections to it as required.
  * When a connection is added (by specifying a server URL),
  * the server's list of supported bridge types is
  * made available locally.
  * Bridge types registered this way will include the server
  * URL and remote bridge type.
  * Asking an smtk::model::Manager instance to create a bridge
  * of the given name will transparently start a worker using
  * the appropriate server.
  */
class SMTKREMOTE_EXPORT RemusBridgeConnections
{
public:
  smtkTypeMacro(RemusBridgeConnections);
  smtkCreateMacro(RemusBridgeConnections);
  ~RemusBridgeConnections();

  smtk::shared_ptr<RemusBridgeConnection> connectToServer(std::string const& host = std::string(), int port = -1);

protected:
  RemusBridgeConnections();

  class Internal;
  Internal* m_data;
};

  } // namespace model
} // namespace smtk

#endif // __smtk_remote_RemusBridgeConnections_h
