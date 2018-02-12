//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/MeshItem.h"

#include "smtk/io/ExportMesh.h"
#include "smtk/io/ImportMesh.h"
#include "smtk/io/LoadJSON.h"
#include "smtk/io/ModelToMesh.h"

#include "smtk/model/DefaultSession.h"

#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/core/Manager.h"

#include "smtk/mesh/operators/ExportMesh.h"
#include "smtk/model/Manager.h"

#include <fstream>

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

namespace
{
//SMTK_DATA_DIR is a define setup by cmake
std::string write_root = SMTK_SCRATCH_DIR;

void create_simple_mesh_model(smtk::model::ManagerPtr mgr, std::string file_path)
{
  std::ifstream file(file_path.c_str());

  std::string json((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));

  //we should load in the test2D.json file as an smtk to model
  smtk::io::LoadJSON::intoModelManager(json.c_str(), mgr);
  mgr->assignDefaultNames();

  file.close();
}

void cleanup(const std::string& file_path)
{
  //first verify the file exists
  ::boost::filesystem::path path(file_path);
  if (::boost::filesystem::is_regular_file(path))
  {
    //remove the file_path if it exists.
    ::boost::filesystem::remove(path);
  }
}
}

int main(int argc, char* argv[])
{
  if (argc == 1)
  {
    std::cout << "Must provide input file as argument" << std::endl;
    return 1;
  }

  std::ifstream file;
  file.open(argv[1]);
  if (!file.good())
  {
    std::cout << "Could not open file \"" << argv[1] << "\".\n\n";
    return 1;
  }

  // Create a model manager
  smtk::model::ManagerPtr manager = smtk::model::Manager::create();

  // Access the mesh manager
  smtk::mesh::ManagerPtr meshManager = manager->meshes();

  // Load in the model
  create_simple_mesh_model(manager, std::string(argv[1]));

  // Convert it to a mesh
  smtk::io::ModelToMesh convert;
  smtk::mesh::CollectionPtr c = convert(meshManager, manager);

  // Create a new "export mesh" operator
  smtk::operation::Operation::Ptr exportMeshOp = smtk::mesh::ExportMesh::create();
  if (!exportMeshOp)
  {
    std::cerr << "No \"export mesh\" operator\n";
    return 1;
  }

  // Set "export mesh" operator's file
  std::string export_path = std::string(write_root + "/testmesh.2dm");
  exportMeshOp->parameters()->findFile("filename")->setValue(export_path);

  bool valueSet = exportMeshOp->parameters()->findMesh("mesh")->setValue(
    meshManager->collectionBegin()->second->meshes());

  if (!valueSet)
  {
    std::cerr << "Failed to set mesh value on export mesh operator\n";
    return 1;
  }

  if (!exportMeshOp->parameters()->isValid())
  {
    std::cerr << "Invalid parameters\n";
    return 1;
  }

  // Execute "export mesh" operator...
  smtk::operation::Operation::Result exportMeshOpResult = exportMeshOp->operate();
  // ...and test the results for success.
  if (exportMeshOpResult->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    std::cerr << "Export mesh operator failed\n";
    return 1;
  }

  // Grab the original mesh collection
  c = meshManager->collectionBegin()->second;
  if (!c->isModified())
  {
    std::cerr << "collection should be marked as modified" << std::endl;
    return 1;
  }

  cleanup(export_path);

  return 0;
}
