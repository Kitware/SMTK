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

#include "smtk/model/StringData.h"

#include "smtk/common/Paths.h"

#ifndef _MSC_VER
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
#endif
#include "remus/server/Server.h"
#include "remus/server/WorkerFactory.h"
#ifndef _MSC_VER
#  pragma GCC diagnostic pop
#endif

#include "clpp/parser.hpp"

#include <iostream>

using namespace smtk::model;

int usage(
  int errCode = 0, const std::string& msg = std::string())
{
  // I. Basic usage info.
  std::cout
    << "Usage:\n"
    << "  smtk-model-server [options]\n"
    << "where options may include\n"
    << "  -no-default-search  Excludes default RemusWorker (.rw) search directories.\n"
    << "  -search=<dir>       Specifies a path to search for RemusWorker (.rw) files\n"
    << "  -client=<host:port> Specifies the host and port to listen for client connections\n"
    << "  -worker=<host:port> Specifies the host and port to listen for worker connections\n"
    << "\n"
    ;

  // II. Print user-specified message and return exit code.
  if (!msg.empty())
    std::cout << msg << "\n";

  return errCode;
}

// A struct to hold options passed to the model server.
struct ProgOpts
{
  ProgOpts()
    :
      m_printhelp(false), m_clientSet(false), m_workerSet(false),
      m_clientHost("127.0.0.1"), m_clientPort(remus::SERVER_CLIENT_PORT),
      m_workerHost("127.0.0.1"), m_workerPort(remus::SERVER_WORKER_PORT)
    {
    smtk::common::Paths paths;
    this->m_search = paths.workerSearchPaths();
    }

  void setProgPath(const std::string& selfPath) { this->m_progPath = selfPath; }
  void setPrintHelp() { this->m_printhelp = true; }
  void setClient(const std::string& hostport)
    {
    this->convertHostPort(hostport, this->m_clientHost, this->m_clientPort);
    this->m_clientSet = true;
    }
  void setWorker(const std::string& hostport)
    {
    this->convertHostPort(hostport, this->m_workerHost, this->m_workerPort);
    this->m_workerSet = true;
    }
  void clearSearch() { this->m_search.clear(); }
  void addSearch(const std::string& path) { this->m_search.push_back(path); }

  std::string progPath() const { return this->m_progPath; }
  bool printHelp() const { return this->m_printhelp; }
  std::string clientHost() const { return this->m_clientHost; }
  int clientPort() const { return this->m_clientPort; }
  std::string workerHost() const { return this->m_workerHost; }
  int workerPort() const { return this->m_workerPort; }
  const StringList& search() const { return this->m_search; }
  bool clientSet() const { return this->m_clientSet; }
  bool workerSet() const { return this->m_workerSet; }

  void convertHostPort(const std::string& hostport, std::string& host, int& port)
    {
    std::string::size_type pos = hostport.rfind(':');
    if (pos > 0)
      host = hostport.substr(0, pos);
    if (pos + 1 != std::string::npos)
      {
      std::stringstream pstr(hostport.substr(pos + 1));
      pstr >> port;
      }
    }

  bool m_printhelp;
  bool m_clientSet;
  bool m_workerSet;
  std::string m_clientHost;
  int m_clientPort;
  std::string m_workerHost;
  int m_workerPort;
  StringList m_search;
  std::string m_progPath;
};

int main (int argc, char* argv[])
{
  ProgOpts opts;
  opts.setProgPath(argv[0]);
  clpp::command_line_parameters_parser args;
  try
    {
    args.add_parameter("-no-default-search", &opts, &ProgOpts::clearSearch);
    args.add_parameter("-search", &opts, &ProgOpts::addSearch);
    args.add_parameter("-client", &opts, &ProgOpts::setClient);
    args.add_parameter("-worker", &opts, &ProgOpts::setWorker);
    args.parse(argc, argv);
    }
  catch (std::exception& e)
    {
    return usage(1, e.what());
    }
  if (opts.printHelp())
    {
    return usage(0);
    }

  remus::server::ServerPorts ports;
  if (opts.clientSet() || opts.workerSet())
    {
    ports = remus::server::ServerPorts(
      opts.clientHost(), opts.clientPort(),
      opts.workerHost(), opts.workerPort());
    }

  boost::shared_ptr<remus::server::WorkerFactory> factory =
    boost::shared_ptr<remus::server::WorkerFactory>(
      new remus::server::WorkerFactory);
  StringList::const_iterator pathIt;
  for (pathIt = opts.search().begin(); pathIt != opts.search().end(); ++pathIt)
    {
    std::cout << "Looking for workers in " << *pathIt << "\n";
    factory->addWorkerSearchDirectory(*pathIt);
    }
  factory->setMaxWorkerCount(10);
  remus::common::MeshIOTypeSet mtypes = factory->supportedIOTypes();
  remus::common::MeshIOTypeSet::const_iterator it;
  for (it = mtypes.begin(); it != mtypes.end(); ++it)
    std::cout << "  Worker " << it->inputType() << "->" << it->outputType() << "\n";

  std::cout
    << "Listening for clients on " << ports.client().host() << ":" << ports.client().port() << "\n"
    << "Listening for workers on " << ports.worker().host() << ":" << ports.worker().port() << "\n"
    << "...\n";

  remus::server::Server server(ports, factory);
  bool valid = server.startBrokeringWithoutSignalHandling();
  server.waitForBrokeringToFinish();
  return valid ? 0 : 1;
}
