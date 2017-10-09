//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME MeshServerLauncher
// .SECTION Description
// Starts up a thread remus::server and reports back the host name
// and port that the server has bound to

#ifndef __smtk_extension_remus_MeshServerLauncher_h
#define __smtk_extension_remus_MeshServerLauncher_h

#include "smtk/extension/remus/smtkRemusExtExports.h"

#include <string>

namespace smtk
{
namespace mesh
{
class SMTKREMUSEXT_EXPORT MeshServerLauncher
{
public:
  MeshServerLauncher();
  ~MeshServerLauncher();

  //create a MeshServer
  //returns if we launched a server, if a server already exists
  void launch();

  //returns if we have a MeshServer running
  bool isAlive() { return m_alive; }

  //kills the current mesh server if it exists;
  void terminate();

  void addWorkerSearchDirectory(const std::string& directory);

  //returns a valid zmq endpoint string to use for a connection or bind
  const std::string& clientEndpoint() const;

  //returns the host name section of the endpoint
  const std::string& clientHost() const;

  //returns the zmq transport scheme can be tcp, ipc or inproc
  const std::string& clientScheme() const;

  //returns the port number for tcp connections, for other scheme types
  //will return -1
  int clientPort() const;

  //same as above but for worker connection
  const std::string& workerEndpoint() const;
  const std::string& workerHost() const;
  const std::string& workerScheme() const;
  int workerPort() const;

  //returns the number of mesh types currently receivable through the server
  std::size_t numberOfSupportedMeshTypes();

private:
  bool m_alive;
  struct Implementation;
  Implementation* m_implementation;
};
}
}

#endif
