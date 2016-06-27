//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkCMBMeshServerLauncher.h"

#include "vtkObjectFactory.h"

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

#include <vector>
#include <string>
#include <vtksys/SystemTools.hxx>

#include <remus/common/LocateFile.h>
#include <remus/server/Server.h>
#include <remus/server/WorkerFactory.h>

namespace
{
//the goal here is to find all the places that the mesh workers config
//files could be located and give them to the factory.
inline void locationsToSearchForWorkers(
                      boost::shared_ptr<remus::server::WorkerFactory> factory )
{
  //first we search from the executables locations,
  //if than we search the current working directory if it is a different path
  //than the executable location
  std::string execLoc = remus::common::getExecutableLocation();
  factory->addWorkerSearchDirectory(execLoc);

  typedef std::vector<std::string>::const_iterator It;
  std::vector<std::string> locations = remus::common::relativeLocationsToSearch();

  for(It i = locations.begin(); i != locations.end();++i)
    {
    std::stringstream buffer;
    buffer << execLoc << "/" << *i;
    factory->addWorkerSearchDirectory(buffer.str());
    }
}

}

vtkStandardNewMacro(vtkCMBMeshServerLauncher);

//-----------------------------------------------------------------------------
vtkCMBMeshServerLauncher::vtkCMBMeshServerLauncher()
{
  //we aren't alive till somebody calls launch
  this->Alive = false;

  boost::shared_ptr<remus::server::WorkerFactory> factory(
                                        new remus::server::WorkerFactory());
  factory->setMaxWorkerCount(2);
  locationsToSearchForWorkers(factory);

  //this sets up the server but it isn't accepting any connections,
  //but has bound to the correct ports
  this->Implementation = new remus::server::Server(factory);

  //now setup the server to poll fairly fast, so that when we ask it shutdown,
  //it can in a fairly timely manner. Range is 32ms to 2.5sec for our polling
  remus::server::PollingRates rates(32,2500);
  this->Implementation->pollingRates(rates); //update the server rates

  //launch the server, this must be done before we grab the port info
  this->Launch();

  //grab the host and port info for the client side
  const remus::server::ServerPorts& pinfo =
                                    this->Implementation->serverPortInfo();
  this->HostName = pinfo.client().host();
  this->PortNumber = pinfo.client().port();
}

//-----------------------------------------------------------------------------
vtkCMBMeshServerLauncher::~vtkCMBMeshServerLauncher()
{
  this->Terminate();
  delete this->Implementation;
  this->Implementation = NULL;
}

//-----------------------------------------------------------------------------
int vtkCMBMeshServerLauncher::Launch()
{
  //remus can handle being asked to start brokering even when it is
  //currently brokering
  this->Implementation->startBrokeringWithoutSignalHandling();
  this->Alive = true;
  return 1;
}

//-----------------------------------------------------------------------------
int vtkCMBMeshServerLauncher::IsAlive()
{
  return !!this->Alive;
}

//-----------------------------------------------------------------------------
int vtkCMBMeshServerLauncher::Terminate()
{
  if(this->Alive)
    {
    this->Implementation->stopBrokering();
    this->Alive = false;
    }
  return 1;
}

//-----------------------------------------------------------------------------
void vtkCMBMeshServerLauncher::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Host Name: " << this->HostName << std::endl;
  os << indent << "Port Number: " << this->PortNumber << std::endl;
  os << indent << "Alive: " << this->Alive << std::endl;
}
