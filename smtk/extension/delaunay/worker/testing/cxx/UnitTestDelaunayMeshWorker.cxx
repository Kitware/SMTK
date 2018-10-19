//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "remus/common/ContentTypes.h"
#include "remus/common/LocateFile.h"
#include "remus/common/MeshIOType.h"
#include "remus/proto/JobRequirements.h"

#include "smtk/AutoInit.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/session/polygon/Operation.h"
#include "smtk/session/polygon/Resource.h"

#include "smtk/common/Paths.h"

#include "smtk/extension/remus/MeshOperation.h"
#include "smtk/extension/remus/MeshServerLauncher.h"

#include "smtk/io/AttributeReader.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/ExportMesh.h"
#include "smtk/io/Logger.h"

#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/utility/ExtractTessellation.h"

#include "smtk/model/Edge.h"
#include "smtk/model/EntityIterator.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Loop.h"

#include "smtk/operation/operators/ReadResource.h"

#include "smtk/resource/Manager.h"

#include "smtk/operation/Manager.h"

#include <fstream>

namespace
{

const std::vector<std::string>& relative_search_paths()
{
  static std::vector<std::string> rel_search_paths = { "." };
  return rel_search_paths;
}

const std::vector<std::string> absolute_search_paths()
{
  static smtk::common::Paths paths;
  return paths.workerSearchPaths();
}
}

int main(int argc, char** const argv)
{
  std::cout << "This test has been disabled until mesh workers have been updated." << std::endl;
  return 125;

  if (argc == 1)
  {
    std::cout << "Usage: UnitTestDelaunayMesh <polygon_model.smtk>" << std::endl;
    return 0;
  }

  // Locate our input file from the input
  std::string file_path(argv[1]);

  // Ensure that the file is valid
  std::ifstream file(file_path.c_str());
  if (!file.good())
  {
    std::cerr << "Cannot read input file" << std::endl;
    file.close();
    return 1;
  }
  file.close();

  // Create a resource manager
  smtk::resource::Manager::Ptr resourceManager = smtk::resource::Manager::create();

  // Register the resources to the resource manager
  {
    resourceManager->registerResource<smtk::model::Resource>();
    resourceManager->registerResource<smtk::session::polygon::Resource>();
  }

  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  // Register the operators to the operation manager
  {
    operationManager->registerOperation<smtk::extension::remus::MeshOperation>(
      "smtk::extension::remus::MeshOperation");
  }

  // Register the resource manager to the operation manager (newly created
  // resources will be automatically registered to the resource manager).
  operationManager->registerResourceManager(resourceManager);

  smtk::model::Entity::Ptr model;

  {
    // Create an import operator
    smtk::operation::ReadResource::Ptr loadOp =
      operationManager->create<smtk::operation::ReadResource>();
    if (!loadOp)
    {
      std::cerr << "No load operator\n";
      return 1;
    }

    loadOp->parameters()->findFile("filename")->setValue(file_path.c_str());
    smtk::operation::ReadResource::Result result = loadOp->operate();
    if (result->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      std::cerr << "Could not load smtk model!\n";
      return 1;
    }
    model =
      std::dynamic_pointer_cast<smtk::model::Entity>(result->findComponent("created")->value());
  }

  // Start an instance of our mesh server
  smtk::mesh::MeshServerLauncher meshServerLauncher;

  // Provide the worker factory with a list of locations where it can
  // find the job requirements file (*.rw) and associated executable.
  smtk::common::Paths paths;
  for (std::string path : paths.workerSearchPaths())
  {
    meshServerLauncher.addWorkerSearchDirectory(path);
  }

  // Construct a mesh operator
  smtk::extension::remus::MeshOperation::Ptr mesher =
    operationManager->create<smtk::extension::remus::MeshOperation>();
  if (!mesher)
  {
    std::cerr << "No mesh operator\n";
    return 1;
  }

  // Set the input model to be meshed
  //
  // TODO: make this an association, not an item
  mesher->parameters()->findComponent("model")->setValue(model);

  // Set the client endpoint for the operator
  mesher->parameters()->findString("endpoint")->setValue(meshServerLauncher.clientEndpoint());

  // Set the job requirements for the operator
  {
    remus::proto::JobRequirements reqs = remus::proto::make_JobRequirements(
      remus::common::make_MeshIOType(remus::meshtypes::Model(), remus::meshtypes::Model()),
      std::string("DelaunayMeshWorker"), remus::common::findFile("DelaunayMeshingDefs", "sbt",
                                           relative_search_paths(), absolute_search_paths()),
      remus::common::ContentFormat::XML);
    std::stringstream s;
    s << reqs;
    mesher->parameters()->findString("remusRequirements")->setValue(s.str());
  }

  // Set the meshing attributes for the operator
  std::string meshingAttributesStr;
  {
    smtk::attribute::ResourcePtr meshingAttributes = smtk::attribute::Resource::create();
    smtk::io::Logger logger;
    smtk::io::AttributeReader reader;
    reader.read(meshingAttributes, remus::common::findFile("DelaunayMeshingDefs", "sbt",
                                     relative_search_paths(), absolute_search_paths())
                                     .path(),
      logger);

    // The mesher may require an attribute of type "Globals" that is named
    // "Globals". When run through ModelBuilder, this attribute is created
    // automatically as an instanced attribute in a view. Since we do not
    // have any view to the operator attributes, we must instantiate this
    // attribute ourselves.
    //
    // TODO: the construction of default attributes should be an automated
    //       process that is callable within the attribute resource.
    meshingAttributes->createAttribute("Globals", "Globals");
    smtk::io::AttributeWriter writer;
    writer.writeContents(meshingAttributes, meshingAttributesStr, logger);

    mesher->parameters()->findString("meshingControlAttributes")->setValue(meshingAttributesStr);
  }

  // Confirm the state of the operator
  if (!mesher->ableToOperate())
  {
    std::cerr << "Mesh operator cannot operate\n";
    meshServerLauncher.terminate();
    return 1;
  }

  // Execute the mesh operator
  smtk::extension::remus::MeshOperation::Result result = mesher->operate();

  // We no longer need the mesh server, so we terminate it
  meshServerLauncher.terminate();

  // Check the outcome of the operation
  if (result->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    std::cerr << "Mesh operator failed\n";
    return 1;
  }

  // Access the face that was meshed
  smtk::model::Face face = model->modelResource()->findEntitiesOfType(smtk::model::FACE)[0];

  // Access the mesh resource associated with the model
  smtk::mesh::CollectionPtr triangulatedFace =
    model->modelResource()->meshes()->associatedCollections(face)[0];

  // confirm that the number of points and cells match our expectations
  if (triangulatedFace->points().size() != 12)
  {
    std::cerr << "Incorrect number of points\n";
    return 1;
  }

  if (triangulatedFace->cells().size() != 10)
  {
    std::cerr << "Incorrect number of cells\n";
    return 1;
  }

  return 0;
}
