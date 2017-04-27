//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef SHIBOKEN_SKIP
#include "smtk/bridge/remote/RemusConnections.h"
#include "smtk/bridge/remote/RemusConnection.h"

#include "remus/client/ServerConnection.h"

#include <map>

namespace smtk
{
namespace bridge
{
namespace remote
{

typedef smtk::shared_ptr<smtk::bridge::remote::RemusConnection> ClientConnectionType;

struct RemusServerDetails
{
  ClientConnectionType connection;
};

typedef std::map<std::string, RemusServerDetails> ConnectionMapType;

class RemusConnections::Internal
{
public:
  ConnectionMapType connections;
};

RemusConnections::RemusConnections()
{
  this->m_data = new Internal;
}

RemusConnections::~RemusConnections()
{
  delete this->m_data;
}

/**\brief Connect to an SMTK model server at the given \a host and \a port.
  *
  * The \a host string should either be empty or a hostname or an IP address.
  * An empty hostname indicates that a local, in-process server should be used.
  *
  * The \a  port is an optional integer. If negative, the default port will be used.
  */
RemusConnection::Ptr RemusConnections::connectToServer(const std::string& host, int port)
{
  std::string url;
  if (!host.empty())
  {
    std::ostringstream urlSS;
    urlSS << host;
    if (port > 0)
      urlSS << ":" << port;
    url = urlSS.str();
  }

  ConnectionMapType::iterator it = this->m_data->connections.find(url);
  if (it != this->m_data->connections.end())
    return it->second.connection;

  RemusConnection::Ptr conn = RemusConnection::create();
  if (!url.empty())
    conn->connectToServer(host, port);
  this->m_data->connections[url].connection = conn;

  // TODO: ask server for supported worker types then ask each worker for
  //       its list of sessions and their file types. Finally, register
  //       each session type under an alias.
  return conn;
}

} // namespace remote
} // namespace bridge
} // namespace smtk
#endif // SHIBOKEN_SKIP
