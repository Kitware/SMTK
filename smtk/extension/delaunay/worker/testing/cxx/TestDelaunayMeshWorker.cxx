//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "remus/client/Client.h"
#include "remus/common/LocateFile.h"
#include "remus/common/SleepFor.h"
#include "remus/server/Server.h"
#include "remus/server/WorkerFactory.h"
#include "remus/worker/Worker.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/bridge/polygon/Session.h"
#include "smtk/io/LoadJSON.h"
#include "smtk/io/SaveJSON.h"
#include "smtk/model/Manager.h"
#include "smtk/model/operators/LoadSMTKModel.h"

#include <fstream>
#include <iostream>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include "smtk/extension/delaunay/worker/DelaunayMeshWorker.h"

namespace factory
{
//we want a custom factory that can not create any workers
class NoSupportFactory : public remus::server::WorkerFactory
{
public:
  remus::proto::JobRequirementsSet workerRequirements(remus::common::MeshIOType type) const override
  { //return empty
    (void)type;
    return remus::proto::JobRequirementsSet();
  }

  bool haveSupport(const remus::proto::JobRequirements& reqs) const override
  {
    (void)reqs;
    //we want to return true here so that the server always queues
    return true;
  }

  bool createWorker(const remus::proto::JobRequirements& type,
    WorkerFactory::FactoryDeletionBehavior lifespan) override
  {
    (void)type;
    (void)lifespan;
    //we want to return false here so that server never thinks we are creating
    //a worker and assigns a job to a worker we didn't create
    return false;
  }
};
}

namespace worker
{

class workerHandle
{
public:
  workerHandle(remus::worker::ServerConnection conn, remus::common::FileHandle const& fhandle)
    : worker(new DelaunayMeshWorker(conn, fhandle))
    , thread()
  {
  }

  void meshJob()
  {
    boost::shared_ptr<boost::thread> workerThread(
      new boost::thread(&DelaunayMeshWorker::meshJob, worker.get()));
    thread.swap(workerThread);

    //now sleep for a couple milliseconds to let the background server
    //and worker threads talk to each other
    remus::common::SleepForMillisec(125);
  }

  boost::shared_ptr<DelaunayMeshWorker> worker;
  boost::shared_ptr<boost::thread> thread;
};
}

namespace
{

//------------------------------------------------------------------------------
//construct a smtk::model::Manager and load all the model from a file
//into the manager
smtk::model::ManagerPtr create_polygon_model(const std::string file_path)
{
  // Create an import operator
  smtk::model::LoadSMTKModel::Ptr loadOp = smtk::model::LoadSMTKModel::create();
  if (!loadOp)
  {
    std::cerr << "No load operator\n";
    return smtk::model::ManagerPtr();
  }

  loadOp->parameters()->findFile("filename")->setValue(file_path.c_str());
  smtk::model::LoadSMTKModel::Result result = loadOp->operate();
  if (result->findInt("outcome")->value() !=
    static_cast<int>(smtk::model::LoadSMTKModel::Outcome::SUCCEEDED))
  {
    std::cerr << "Could not load smtk model!\n";
    return smtk::model::ManagerPtr();
  }

  return std::dynamic_pointer_cast<smtk::model::Manager>(result->findResource("resource")->value());
}

//------------------------------------------------------------------------------
//construct a remus server and start brokering
//return a shared ptr of the server since remus::Server can't be copied
boost::shared_ptr<remus::Server> make_Server(remus::server::ServerPorts ports)
{
  boost::shared_ptr<factory::NoSupportFactory> factory(new factory::NoSupportFactory());
  factory->setMaxWorkerCount(1);

  boost::shared_ptr<remus::Server> server(new remus::Server(ports, factory));
  server->startBrokeringWithoutSignalHandling();
  return server;
}

//------------------------------------------------------------------------------
//construct a client that binds to a given server client port, needs to share
//the same context as the server as we are in the same thread
//return a shared ptr of the client since remus::Client can't be copied
boost::shared_ptr<remus::Client> make_Client(const remus::server::ServerPorts& ports)
{
  remus::client::ServerConnection conn =
    remus::client::make_ServerConnection(ports.client().endpoint());
  boost::shared_ptr<remus::Client> c(new remus::client::Client(conn));
  return c;
}

//------------------------------------------------------------------------------
//construct a worker that binds to a given server worker port, needs to share
//the same context as the server as we are in the same thread
worker::workerHandle make_DelaunayWorker(const remus::server::ServerPorts& ports)
{
  remus::worker::ServerConnection conn =
    remus::worker::make_ServerConnection(ports.worker().endpoint());

  //currently needed to work around a bug in the worker context/socket
  //ownership implementation
  conn.context(ports.context());

  //this is a file based mesher, we need to determine the location of the
  //file containing the xml smtk attributes. We know it should be in the exact
  //same folder as the executable, and we know its name, that should be enough.
  remus::common::FileHandle rfile(remus::common::findFile("DelaunayMeshingDefs", "sbt"));

  //create a worker handle that holds onto the worker and a thread,
  //so that we can mesh a job on request in a background thread.
  return worker::workerHandle(conn, rfile);
}

//------------------------------------------------------------------------------
//Given a remus client and a mesh IO type return the requirements for that worker
//doesn't handle the use case that we can't find any requirements for the
//given mesh types
remus::proto::JobRequirements find_Requirements(
  boost::shared_ptr<remus::Client> client, remus::common::MeshIOType mtype, std::string workerName)
{
  remus::proto::JobRequirementsSet reqSet = client->retrieveRequirements(mtype);

  typedef remus::proto::JobRequirementsSet::const_iterator cit;
  for (cit i = reqSet.begin(); i != reqSet.end(); ++i)
  {
    if (i->workerName() == workerName)
    {
      return *i;
    }
  }
  return remus::proto::JobRequirements();
}
}

//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cout << "TestDelaunayMeshWorker <input_polygon_file>" << std::endl;
    return 0;
  }
  std::string input_file(argv[1]);

  //construct a simple server,client, and worker which shares the same context as they
  //are in the same thread. We grab the server ports instance from the server
  //after construction because, the ports we request to bind to might already be
  //bound by another process, and remus will instead find other ports to bind too
  boost::shared_ptr<remus::Server> server = make_Server(remus::server::ServerPorts());
  const remus::server::ServerPorts& ports = server->serverPortInfo();

  worker::workerHandle worker = make_DelaunayWorker(ports);
  //now that we have a server and worker created lets go ahead
  //and get the worker to start looking for a job
  worker.meshJob(); //non-blocking

  //construct the client to submit a job to that worker
  boost::shared_ptr<remus::Client> client = make_Client(ports);

  remus::common::MeshIOType meshType((remus::meshtypes::Model()), (remus::meshtypes::Model()));

  const bool serverSupportsMeshType = client->canMesh(meshType);
  if (!serverSupportsMeshType)
  {
    std::cerr << "Server is unable to provide a delaunay worker." << std::endl;
    return 1;
  }

  //get the requirements from the delaunay worker
  const std::string workerName("DelaunayMeshWorker");
  remus::proto::JobRequirements delaunayReqs = find_Requirements(client, meshType, workerName);
  if (delaunayReqs.workerName() != workerName)
  {
    std::cerr << "Server is unable to provide a delaunay worker,"
              << "but it has other model to model workers "
              << "(" << delaunayReqs.workerName() << ")." << std::endl;

    return 1;
  }

  //construct a JobSubmission
  remus::proto::JobSubmission submission(delaunayReqs);

  //Now we need to load up a smtk::model from file
  smtk::model::ManagerPtr manager = create_polygon_model(input_file);
  if (!manager)
  {
    std::cerr << "Unable to load the model file from disk." << std::endl;
    return 1;
  }

  //Second step it serialize this manager
  std::string serializedModel = smtk::io::SaveJSON::fromModelManager(manager);

  //construct a zero copy job content of the serializedModel labelling it as
  //JSON encoded data
  submission["model"] = remus::proto::JobContent(
    remus::common::ContentFormat::JSON, serializedModel.c_str(), serializedModel.size());

  //submit the job to the server, getting back a reference we can use
  //to query on the status of the work
  remus::proto::Job jobInfo = client->submitJob(submission);

  //abuse busy-wait looping
  remus::proto::JobStatus currentJobStatus = client->jobStatus(jobInfo);
  while (currentJobStatus.good())
  {
    //now sleep for a bit so we don't hammer the server super hard
    //todo: need to use the status stream for monitoring
    remus::common::SleepForMillisec(16);
    currentJobStatus = client->jobStatus(jobInfo);
  }

  //determine if the job was successfull
  const bool haveFinishedJob = currentJobStatus.finished();

  if (!haveFinishedJob)
  {
    std::cerr << "Job was submitted, but didn't successfully complete." << std::endl;
    std::cerr << "[remus] Final Status of: " << remus::proto::to_string(jobInfo)
              << "is: " << remus::to_string(currentJobStatus.status()) << std::endl;
    return 1;
  }

  //grab the results from the worker
  remus::proto::JobResult meshMetaData = client->retrieveResults(jobInfo);

  //todo: convert the meshMetaData back into model/mesh
  std::cout << std::string(meshMetaData.data(), meshMetaData.dataSize()) << std::endl;

  return 0;
}
