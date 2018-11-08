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
#include "smtk/io/ModelToMesh.h"

#include "smtk/model/DefaultSession.h"
#include "smtk/model/json/jsonResource.h"

#include "smtk/mesh/core/Component.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/mesh/operators/Export.h"
#include "smtk/model/Resource.h"

#include <fstream>

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

namespace
{
//SMTK_DATA_DIR is a define setup by cmake
std::string write_root = SMTK_SCRATCH_DIR;

void create_simple_mesh_model(smtk::model::ResourcePtr resource, std::string file_path)
{
  std::ifstream file(file_path.c_str());

  std::string json_str((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
  nlohmann::json json = nlohmann::json::parse(json_str);

  smtk::model::from_json(json, resource);
  for (auto& tessPair : json["tessellations"])
  {
    smtk::common::UUID id = tessPair[0];
    smtk::model::Tessellation tess = tessPair[1];
    resource->setTessellation(id, tess);
  }

  resource->assignDefaultNames();

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

  // Create a model resource
  smtk::model::ResourcePtr resource = smtk::model::Resource::create();

  // Load in the model
  create_simple_mesh_model(resource, std::string(argv[1]));

  // Convert it to a mesh
  smtk::io::ModelToMesh convert;
  smtk::mesh::ResourcePtr meshResource = convert(resource);

  // Create a new "export mesh" operator
  smtk::operation::Operation::Ptr exportMeshOp = smtk::mesh::Export::create();
  if (!exportMeshOp)
  {
    std::cerr << "No \"export mesh\" operator\n";
    return 1;
  }

  // Set "export mesh" operator's file
  std::string export_path = std::string(write_root + "/testmesh.2dm");
  exportMeshOp->parameters()->findFile("filename")->setValue(export_path);

  bool valueSet =
    exportMeshOp->parameters()->associate(smtk::mesh::Component::create(meshResource->meshes()));

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

  // Test the original mesh resource
  if (!meshResource->isModified())
  {
    std::cerr << "mesh resource should be marked as modified" << std::endl;
    return 1;
  }

  cleanup(export_path);

  return 0;
}
