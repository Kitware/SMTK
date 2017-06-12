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

#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/System.h"

#include "smtk/bridge/polygon/Operator.h"
#include "smtk/bridge/polygon/Session.h"

#include "smtk/extension/remus/MeshServerLauncher.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/ExtractTessellation.h"
#include "smtk/mesh/Manager.h"

#include "smtk/model/Edge.h"
#include "smtk/model/EntityIterator.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/FaceUse.h"
#include "smtk/model/Loop.h"

#include "smtk/io/AttributeReader.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/ExportMesh.h"
#include "smtk/io/Logger.h"
#include "smtk/io/ModelToMesh.h"

#include <fstream>

namespace
{

const std::vector<std::string>& relative_search_paths()
{
  static std::vector<std::string> rel_search_paths = { "." };
  return rel_search_paths;
}

const std::vector<std::string>& absolute_search_paths()
{
  static std::vector<std::string> abs_search_paths = { INSTALL_SEARCH_PATH, BUILD_SEARCH_PATH };
  return abs_search_paths;
}
}

int main(int argc, char** const argv)
{
  if (argc == 1)
  {
    std::cout << "Usage: UnitTestDelaunayMesh <polygon_model.smtk>" << std::endl;
    return 0;
  }

  // Create model manager
  smtk::model::ManagerPtr modelManager = smtk::model::Manager::create();

  // Access mesh manager
  smtk::mesh::ManagerPtr meshManager = modelManager->meshes();

  // Create a polygon session and set it as the active session
  smtk::bridge::polygon::Session::Ptr session = smtk::bridge::polygon::Session::create();
  modelManager->registerSession(session);

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

  // Import the file and access the resulting model
  smtk::model::Operator::Ptr op = session->op("load smtk model");

  op->findFile("filename")->setValue(file_path.c_str());
  smtk::model::OperatorResult result = op->operate();
  if (result->findInt("outcome")->value() != smtk::model::OPERATION_SUCCEEDED)
  {
    std::cerr << "Could not load smtk model!\n";
    return 1;
  }
  smtk::model::Model model = result->findModelEntity("created")->value();

  // Ensure that the model is valid
  if (!model.isValid())
  {
    std::cerr << "Model is invalid!" << std::endl;
    return 1;
  }

  // Start an instance of our mesh server
  smtk::mesh::MeshServerLauncher meshServerLauncher;

  // Provide the worker factory with a list of locations where it can
  // find the job requirements file (*.rw) and associated executable.
  // The first path is the install location of the worker, and the second
  // path is the build location.
  for (std::string path : absolute_search_paths())
  {
    meshServerLauncher.addWorkerSearchDirectory(path);
  }

  // Construct a mesh operator
  smtk::model::OperatorPtr mesher = session->op("mesh");
  if (!mesher)
  {
    std::cerr << "No mesh operator\n";
    return 1;
  }

  // Set the input model to be meshed
  //
  // TODO: make this an association, not an item
  mesher->specification()->findModelEntity("model")->setValue(model);

  // Set the client endpoint for the operator
  mesher->specification()->findString("endpoint")->setValue(meshServerLauncher.clientEndpoint());

  // Set the job requirements for the operator
  {
    remus::proto::JobRequirements reqs = remus::proto::make_JobRequirements(
      remus::common::make_MeshIOType(remus::meshtypes::Model(), remus::meshtypes::Model()),
      std::string("DelaunayMeshWorker"), remus::common::findFile("DelaunayMeshingDefs", "sbt",
                                           relative_search_paths(), absolute_search_paths()),
      remus::common::ContentFormat::XML);
    std::stringstream s;
    s << reqs;
    mesher->specification()->findString("remusRequirements")->setValue(s.str());
  }

  // Set the meshing attributes for the operator
  std::string meshingAttributesStr;
  {
    smtk::attribute::SystemPtr meshingAttributes = smtk::attribute::System::create();
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
    //       process that is callable within the atribute system.
    meshingAttributes->createAttribute("Globals", "Globals");
    smtk::io::AttributeWriter writer;
    writer.writeContents(meshingAttributes, meshingAttributesStr, logger);

    mesher->specification()->findString("meshingControlAttributes")->setValue(meshingAttributesStr);
  }

  // Confirm the state of the operator
  if (!mesher->ableToOperate())
  {
    std::cerr << "Mesh operator cannot operate\n";
    meshServerLauncher.terminate();
    return 1;
  }

  // Execute the mesh operator
  result = mesher->operate();

  // We no longer need the mesh server, so we terminate it
  meshServerLauncher.terminate();

  // Check the outcome of the operation
  if (result->findInt("outcome")->value() != smtk::model::OPERATION_SUCCEEDED)
  {
    std::cerr << "Mesh operator failed\n";
    return 1;
  }

  // Access the face that was meshed
  smtk::model::Face face = modelManager->findEntitiesOfType(smtk::model::FACE)[0];

  // Access the mesh collection associated with the model
  smtk::mesh::CollectionPtr triangulatedFace = meshManager->associatedCollections(face)[0];

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

smtkComponentInitMacro(smtk_remus_mesh_operator);
smtkComponentInitMacro(smtk_polygon_session);
