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

#include "smtk/io/ImportMesh.h"

#include "smtk/model/DefaultSession.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Operator.h"

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

namespace
{
//SMTK_DATA_DIR is a define setup by cmake
std::string write_root = SMTK_SCRATCH_DIR;

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

    // Create a model manager
    smtk::model::ManagerPtr manager = smtk::model::Manager::create();

    // Identify available sessions
    std::cout << "Available sessions\n";
    typedef smtk::model::StringList StringList;
    StringList sessions = manager->sessionTypeNames();
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

    // Create a new "import" operator
    smtk::model::OperatorPtr importOp = sessRef.session()->
      op("import smtk model");
    if (!importOp)
      {
      std::cerr << "No \"import\" operator\n";
      return 1;
      }

    // Set "import" operator's file to a test file
    importOp->specification()->findFile("filename")->
      setValue(std::string(argv[1]));

    std::cout<<std::string(argv[1])<<std::endl;

    // Execute "import" operator...
    smtk::model::OperatorResult importOpResult = importOp->operate();
    // ...and test the results for success.
    if (importOpResult->findInt("outcome")->value() !=
        smtk::model::OPERATION_SUCCEEDED)
      {
      std::cerr << "Import operator failed\n";
      return 1;
      }

    // Test all three mesh file types
    std::string extension[3] = {".h5m",".vtk",".exo"};

    for (int fileType = 0; fileType < 3; ++fileType)
      {
      std::cout<<"Testing file type "<<extension[fileType]<<std::endl;

      // Create a new "export mesh" operator
      smtk::model::OperatorPtr exportMeshOp = sessRef.session()->
        op("export mesh");
      if (!exportMeshOp)
        {
        std::cerr << "No \"export mesh\" operator\n";
        return 1;
        }

      // Set "export mesh" operator's file
      std::string write_path = std::string(write_root + "/testmesh" +
                                           extension[fileType]);
      exportMeshOp->specification()->findFile("filename")->
        setValue(write_path);

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
      smtk::mesh::CollectionPtr c = sessRef.session()->meshManager()->
        collectionBegin()->second;

      // Reload the written file and verify the number of meshes are the same as
      // the input mesh
      smtk::mesh::CollectionPtr c2 =
        smtk::io::ImportMesh::entireFile(write_path,
                                         sessRef.session()->meshManager());
      if( c2->isModified() )
        {
        std::cerr<<"collection shouldn't be marked as modified"<<std::endl;
        return 1;
        }

      // Remove the file from disk
      cleanup( write_path );

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

  return 0;
}
