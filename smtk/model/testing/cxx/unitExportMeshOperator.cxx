//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/MeshItem.h"

#include "smtk/io/ExportMesh.h"
#include "smtk/io/ImportJSON.h"
#include "smtk/io/ImportMesh.h"
#include "smtk/io/ModelToMesh.h"

#include "smtk/model/DefaultSession.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Operator.h"

#include <fstream>

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

namespace
{
//SMTK_DATA_DIR is a define setup by cmake
std::string write_root = SMTK_SCRATCH_DIR;

void create_simple_mesh_model( smtk::model::ManagerPtr mgr,
                               std::string file_path )
{
  std::ifstream file(file_path.c_str());

  std::string json(
    (std::istreambuf_iterator<char>(file)),
    (std::istreambuf_iterator<char>()));

  //we should load in the test2D.json file as an smtk to model
  smtk::io::ImportJSON::intoModelManager(json.c_str(), mgr);
  mgr->assignDefaultNames();

  file.close();
}

void cleanup( const std::string& file_path )
{
  //first verify the file exists
  ::boost::filesystem::path path( file_path );
  if( ::boost::filesystem::is_regular_file( path ) )
    {
    //remove the file_path if it exists.
    ::boost::filesystem::remove( path );
    }
}

}

int main(int argc, char* argv[])
{
  if (argc == 1)
    {
    std::cout<<"Must provide input file as argument"<<std::endl;
    return 1;
    }

  std::ifstream file;
  file.open(argv[1]);
  if(!file.good())
    {
    std::cout
      << "Could not open file \"" << argv[1] << "\".\n\n";
      return 1;
    }

  std::vector<std::string> files_to_delete;

  // Create a model manager
  smtk::model::ManagerPtr manager = smtk::model::Manager::create();

  // Identify available sessions
  std::cout << "Available sessions\n";
  typedef smtk::model::StringList StringList;
  StringList sessions = manager->sessionTypeNames();
  smtk::mesh::ManagerPtr meshManager = manager->meshes();
  for (StringList::iterator it = sessions.begin(); it != sessions.end(); ++it)
    std::cout << "  " << *it << "\n";
  std::cout << "\n";

  // Create a new default session
  smtk::model::SessionRef sessRef = manager->createSession("native");

  // Identify available operators
  std::cout << "Available cmb operators\n";
  StringList opnames = sessRef.session()->operatorNames();
  for (StringList::iterator it = opnames.begin(); it != opnames.end(); ++it)
    {
    std::cout << "  " << *it << "\n";
    }
  std::cout << "\n";

  create_simple_mesh_model( manager, std::string(argv[1]) );
  smtk::io::ModelToMesh convert;
  smtk::mesh::CollectionPtr c = convert(meshManager,manager);

  // Create a new "export mesh" operator
  smtk::model::OperatorPtr exportMeshOp = sessRef.session()->
    op("export mesh");
  if (!exportMeshOp)
    {
    std::cerr << "No \"export mesh\" operator\n";
    return 1;
    }

  // Set "export mesh" operator's file
  std::string export_path = std::string(write_root + "/testmesh.2dm");
  exportMeshOp->specification()->findFile("filename")->
    setValue(export_path);

  bool valueSet = exportMeshOp->specification()->findMesh("mesh")->
    setValue(meshManager->collectionBegin()->second->meshes());

  if (!valueSet)
    {
    std::cerr << "Failed to set mesh value on export mesh operator\n";
    return 1;
    }

  // Execute "export mesh" operator...
  smtk::model::OperatorResult exportMeshOpResult = exportMeshOp->operate();
  // ...and test the results for success.
  if (exportMeshOpResult->findInt("outcome")->value() !=
      smtk::model::OPERATION_SUCCEEDED)
    {
    std::cerr << "Export mesh operator failed\n";
    return 1;
    }

  // Grab the original mesh collection
  c = sessRef.session()->meshManager()->collectionBegin()->second;
  if( !c->isModified() )
    {
    std::cerr<<"collection should be marked as modified"<<std::endl;
    return 1;
    }

  cleanup( export_path );

  return 0;
}
