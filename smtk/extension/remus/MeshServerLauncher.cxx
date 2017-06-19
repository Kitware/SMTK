//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "MeshServerLauncher.h"

#include <string>
#include <vector>

#include <remus/common/LocateFile.h>
#include <remus/server/Server.h>
#include <remus/server/WorkerFactory.h>

namespace
{
//the goal here is to find all the places that the mesh workers config
//files could be located and give them to the factory.
inline void locationsToSearchForWorkers(boost::shared_ptr<remus::server::WorkerFactory> factory)
{
  //first we search from the executables locations,
  //if than we search the current working directory if it is a different path
  //than the executable location
  std::string execLoc = remus::common::getExecutableLocation();
  factory->addWorkerSearchDirectory(execLoc);

  typedef std::vector<std::string>::const_iterator It;
  std::vector<std::string> locations = remus::common::relativeLocationsToSearch();

  for (It i = locations.begin(); i != locations.end(); ++i)
  {
    std::stringstream buffer;
    buffer << execLoc << "/" << *i;
    factory->addWorkerSearchDirectory(buffer.str());
  }
}
}

namespace smtk
{
namespace mesh
{

struct MeshServerLauncher::Implementation
{
  Implementation()
    : m_factory(new remus::server::WorkerFactory())
    , m_server(new remus::server::Server(m_factory))
  {
  }

  boost::shared_ptr<remus::server::WorkerFactory> m_factory;
  boost::shared_ptr<remus::server::Server> m_server;
};

MeshServerLauncher::MeshServerLauncher()
{
  //we aren't alive till somebody calls launch
  this->m_alive = false;

  this->m_implementation = new Implementation();

  this->m_implementation->m_factory->setMaxWorkerCount(2);
  locationsToSearchForWorkers(this->m_implementation->m_factory);

  //now setup the server to poll fairly fast, so that when we ask it shutdown,
  //it can in a fairly timely manner. Range is 32ms to 2.5sec for our polling
  remus::server::PollingRates rates(32, 2500);
  this->m_implementation->m_server->pollingRates(rates); //update the server rates

  //launch the server, this must be done before we grab the port info
  this->launch();
}

MeshServerLauncher::~MeshServerLauncher()
{
  this->terminate();
  delete this->m_implementation;
  this->m_implementation = NULL;
}

void MeshServerLauncher::launch()
{
  //remus can handle being asked to start brokering even when it is
  //currently brokering
  this->m_implementation->m_server->startBrokeringWithoutSignalHandling();
  this->m_alive = true;
}

void MeshServerLauncher::terminate()
{
  if (this->m_alive)
  {
    this->m_implementation->m_server->stopBrokering();
    this->m_alive = false;
  }
}

void MeshServerLauncher::addWorkerSearchDirectory(const std::string& directory)
{
  this->m_implementation->m_factory->addWorkerSearchDirectory(directory);
}

const std::string& MeshServerLauncher::clientEndpoint() const
{
  return this->m_implementation->m_server->serverPortInfo().client().endpoint();
}

const std::string& MeshServerLauncher::clientHost() const
{
  return this->m_implementation->m_server->serverPortInfo().client().host();
}

const std::string& MeshServerLauncher::clientScheme() const
{
  return this->m_implementation->m_server->serverPortInfo().client().scheme();
}

int MeshServerLauncher::clientPort() const
{
  return this->m_implementation->m_server->serverPortInfo().client().port();
}

const std::string& MeshServerLauncher::workerEndpoint() const
{
  return this->m_implementation->m_server->serverPortInfo().worker().endpoint();
}

const std::string& MeshServerLauncher::workerHost() const
{
  return this->m_implementation->m_server->serverPortInfo().worker().host();
}

const std::string& MeshServerLauncher::workerScheme() const
{
  return this->m_implementation->m_server->serverPortInfo().worker().scheme();
}

int MeshServerLauncher::workerPort() const
{
  return this->m_implementation->m_server->serverPortInfo().worker().port();
}
}
}
