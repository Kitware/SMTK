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

#include "smtk/io/ImportJSON.h"
#include "smtk/io/ImportMesh.h"
#include "smtk/io/ModelToMesh.h"
#include "smtk/io/WriteMesh.h"

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
  if (argc > 1 )
    {
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

    // Test all three mesh file types
    std::string extension[2] = {".exo", ".h5m"};

    for (int fileType = 0; fileType < 2; ++fileType)
      {
      std::cout<<"Testing file type "<<extension[fileType]<<std::endl;

      // Create a new "write mesh" operator
      smtk::model::OperatorPtr writeMeshOp = sessRef.session()->
        op("write mesh");
      if (!writeMeshOp)
        {
        std::cerr << "No \"write mesh\" operator\n";
        return 1;
        }

      // Set "write mesh" operator's file
      std::string write_path = std::string(write_root + "/testmesh" +
                                           extension[fileType]);
      writeMeshOp->specification()->findFile("filename")->
        setValue(write_path);

      bool valueSet = writeMeshOp->specification()->findMesh("mesh")->
        setValue(meshManager->collectionBegin()->second->meshes());

      if (!valueSet)
        {
        std::cerr << "Failed to set mesh value on write mesh operator\n";
        return 1;
        }

      // Execute "write mesh" operator...
      smtk::model::OperatorResult writeMeshOpResult = writeMeshOp->operate();
      // ...and test the results for success.
      if (writeMeshOpResult->findInt("outcome")->value() !=
          smtk::model::OPERATION_SUCCEEDED)
        {
        std::cerr << "Write mesh operator failed\n";
        return 1;
        }

      // Grab the original mesh collection
      smtk::mesh::CollectionPtr c = sessRef.session()->meshManager()->
        collectionBegin()->second;
      if( c->isModified() )
        {
        std::cerr<<"collection shouldn't be marked as modified"<<std::endl;
        return 1;
        }

      // Reload the written file and verify the number of meshes are the same as
      // the input mesh
      //
      // NB: a new mesh manager must be used here. If the original mesh manager
      //     is used instead, a corrupt collection is added to our manager and
      //     our little trick of grabbing the first collection above will result
      //     in test failures.
      smtk::mesh::ManagerPtr m2 = smtk::mesh::Manager::create();
      smtk::io::ImportMesh import;
      smtk::mesh::CollectionPtr c2 = import( write_path, m2 );
      if( c2->isModified() )
        {
        std::cerr<<"collection shouldn't be marked as modified"<<std::endl;
        return 1;
        }

      // Remove the file from disk
      cleanup(write_path);

      // Verify the meshes
      if (!c2->isValid())
        {
        std::cerr<<"collection should be valid"<<std::endl;
        return 1;
        }
      if (c2->name() != c->name() )
        {
        std::cerr<<"collection names do not match"<<std::endl;
        return 1;
        }

      // We only guarantee that Moab's native .h5m format is bidirectional. The
      // other mesh formats will write, but information is lost when they are
      // subsequently imported.
      if (extension[fileType] == ".h5m")
        {
        if ( c2->numberOfMeshes() != c->numberOfMeshes() )
          {
          std::cerr<<"number of meshes do not match"<<std::endl;
          return 1;
          }
        if ( c2->types() != c->types() )
          {
          std::cerr<<"collection types do not match"<<std::endl;
          return 1;
          }
        }
      }

    }

  return 0;
}
