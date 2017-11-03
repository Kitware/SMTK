//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/common/UUID.h"
#include "smtk/io/ExportMesh.h"
#include "smtk/io/ImportMesh.h"
#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/core/Manager.h"

#include "smtk/mesh/testing/cxx/helpers.h"

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

namespace
{

//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;
std::string write_root = SMTK_SCRATCH_DIR;

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

void verify_write_empty_collection()
{
  std::string write_path(write_root);
  write_path += "/" + smtk::common::UUID::random().toString() + ".2dm";

  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::mesh::CollectionPtr c = manager->makeCollection();
  test(c->isValid(), "empty collection is empty");

  const bool result = smtk::io::exportMesh(write_path, c);

  //before we verify if the write was good, first remove the output file
  cleanup(write_path);
  test(result == false, "nothing to write for an empty collection");
}

void verify_write_null_collection()
{
  std::string write_path(write_root);
  write_path += "/" + smtk::common::UUID::random().toString() + ".2dm";

  //use a null collection ptr
  smtk::mesh::CollectionPtr c;

  const bool result = smtk::io::exportMesh(write_path, c);

  //before we verify if the write was good, first remove the output file
  cleanup(write_path);

  test(result == false, "Can't save null collection to disk");
}

void verify_read_write_valid_collection()
{
  bool result = false;

  std::string file_path(data_root);
  file_path += "/mesh/3d/twoassm_out.h5m";

  std::string write_path(write_root);
  write_path += "/" + smtk::common::UUID::random().toString() + ".2dm";

  {
    smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
    smtk::mesh::CollectionPtr c = smtk::io::importMesh(file_path, manager);
    test(c->isValid(), "collection should be valid");
    test(!c->isModified(), "loaded collection should be marked as not modifed");

    //extract a surface mesh, and write that out
    c->meshes(smtk::mesh::Dims3).extractShell();
    test(c->isModified(), "extractShell should mark the collection as modified");

    if (!smtk::io::exportMesh(write_path, c))
    {
      test(result == true, "failed to properly write out a valid 2dm file");
    }
  }

  {
    smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
    smtk::mesh::CollectionPtr c = smtk::io::importMesh(write_path, manager);
    cleanup(write_path);

    test(c && c->isValid(), "collection should be valid");
    test(!c->isModified(), "loaded collection should be marked as not modifed");

    test(c->meshes().size() == 1, "collection should have 1 mesh");
    test(c->cells().size() == 660, "collection should have 660 cells");
    test(c->points().size() == 662, "collection should have 662 points");
  }
}
}

int UnitTestExportMesh2DM(int, char** const)
{
  verify_write_empty_collection();
  verify_write_null_collection();
  verify_read_write_valid_collection();

  return 0;
}
