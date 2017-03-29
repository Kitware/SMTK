//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_session_remote_RemusConnections_h
#define __smtk_session_remote_RemusConnections_h

#include "smtk/SharedFromThis.h"
#include "smtk/bridge/remote/Exports.h"

#include <string>

namespace smtk {
  namespace bridge {
    namespace remote {

class RemusConnection;

/**\brief Maintain a list of model-server connections.
  *
  * Applications should keep a single instance of this class
  * and add connections to it as required.
  * When a connection is added (by specifying a server URL),
  * the server's list of supported session types is
  * made available locally.
  * Session types registered this way will include the server
  * URL and remote session type.
  * Asking an smtk::model::Manager instance to create a session
  * of the given name will transparently start a worker using
  * the appropriate server.
  */
class SMTKREMOTESESSION_EXPORT RemusConnections
{
public:
  smtkTypeMacro(RemusConnections);
  smtkCreateMacro(RemusConnections);
  ~RemusConnections();

  smtk::shared_ptr<RemusConnection> connectToServer(
    std::string const& host = std::string(), int port = -1);

protected:
  RemusConnections();

  class Internal;
  Internal* m_data;
};

    } // namespace remote
  } // namespace bridge
} // namespace smtk

#endif // __smtk_session_remote_RemusConnections_h
