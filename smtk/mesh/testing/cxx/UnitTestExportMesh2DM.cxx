//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"
#include "smtk/io/ImportMesh.h"
#include "smtk/io/MeshExport2DM.h"

#include "smtk/mesh/testing/cxx/helpers.h"

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;


namespace
{

//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;
std::string write_root = data_root + "/tmp";

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

//----------------------------------------------------------------------------
void verify_write_empty_collection()
{
  std::string write_path(write_root);
  write_path += "/output.2dm";

  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::mesh::CollectionPtr c = manager->makeCollection();
  test( c->isValid(), "empty collection is empty");

  smtk::io::MeshExport2DM exporter;
  const bool result = exporter.write(c, write_path);

  //before we verify if the write was good, first remove the output file
  cleanup( write_path );
  test ( result == false, "nothing to write for an empty collection");
}

//----------------------------------------------------------------------------
void verify_write_null_collection()
{
  std::string write_path(write_root);
  write_path += "/output.2dm";

  //use a null collection ptr
  smtk::mesh::CollectionPtr c;

  smtk::io::MeshExport2DM exporter;
  const bool result = exporter.write(c, write_path);

  //before we verify if the write was good, first remove the output file
  cleanup( write_path );

  test ( result == false, "Can't save null collection to disk");
}

//----------------------------------------------------------------------------
void verify_write_valid_collection()
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/twoassm_out.h5m";

  std::string write_path(write_root);
  write_path += "/twoassm_output.2dm";

  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::mesh::CollectionPtr c = smtk::io::ImportMesh::entireFile(file_path, manager);
  test( c->isValid(), "collection should be valid");
  test( !c->isModified(), "loaded collection should be marked as not modifed");

  //extract a surface mesh, and write that out
  c->meshes(smtk::mesh::Dims3).extractShell();
  test(c->isModified(), "extractShell should mark the collection as modified");

  smtk::io::MeshExport2DM exporter;
  const bool result = exporter.write(c, write_path);
  cleanup( write_path );

  if(!result)
    {
    test( result == true, "failed to properly write out a valid 2dm file");
    }
}

}

//----------------------------------------------------------------------------
int UnitTestExportMesh2DM(int, char** const)
{
  verify_write_empty_collection();
  verify_write_null_collection();
  verify_write_valid_collection();

  return 0;
}
