//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/Options.h"
#include "smtk/SharedPtr.h"

#include "remus/server/Server.h"
#include "remus/server/WorkerFactory.h"

#include <iostream>

int main (int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  remus::server::ServerPorts ports;

  boost::shared_ptr<remus::server::WorkerFactory> factory =
    boost::shared_ptr<remus::server::WorkerFactory>(
      new remus::server::WorkerFactory);
  factory->setMaxWorkerCount(10);

  remus::server::Server server(ports, factory);
  bool valid = server.startBrokeringWithoutSignalHandling();
  server.waitForBrokeringToFinish();
  return valid ? 0 : 1;
}
