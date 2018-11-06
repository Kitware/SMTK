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
#include "smtk/mesh/core/Resource.h"

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

void verify_write_empty_resource()
{
  std::string write_path(write_root);
  write_path += "/" + smtk::common::UUID::random().toString() + ".2dm";

  smtk::mesh::ResourcePtr mr = smtk::mesh::Resource::create();
  test(mr->isValid(), "empty resource is empty");

  const bool result = smtk::io::exportMesh(write_path, mr);

  //before we verify if the write was good, first remove the output file
  cleanup(write_path);
  test(result == false, "nothing to write for an empty resource");
}

void verify_write_null_resource()
{
  std::string write_path(write_root);
  write_path += "/" + smtk::common::UUID::random().toString() + ".2dm";

  //use a null resource ptr
  smtk::mesh::ResourcePtr mr;

  const bool result = smtk::io::exportMesh(write_path, mr);

  //before we verify if the write was good, first remove the output file
  cleanup(write_path);

  test(result == false, "Can't save null resource to disk");
}

void verify_read_write_valid_resource()
{
  bool result = false;

  std::string file_path(data_root);
  file_path += "/mesh/3d/twoassm_out.h5m";

  std::string write_path(write_root);
  write_path += "/" + smtk::common::UUID::random().toString() + ".2dm";

  {
    smtk::mesh::ResourcePtr mr = smtk::mesh::Resource::create();
    smtk::io::importMesh(file_path, mr);
    test(mr->isValid(), "resource should be valid");
    test(!mr->isModified(), "loaded resource should be marked as not modifed");

    //extract a surface mesh, and write that out
    mr->meshes(smtk::mesh::Dims3).extractShell();
    test(mr->isModified(), "extractShell should mark the resource as modified");

    if (!smtk::io::exportMesh(write_path, mr))
    {
      test(result == true, "failed to properly write out a valid 2dm file");
    }
  }

  {
    smtk::mesh::ResourcePtr mr = smtk::mesh::Resource::create();
    smtk::io::importMesh(write_path, mr);
    cleanup(write_path);

    test(mr && mr->isValid(), "resource should be valid");
    test(!mr->isModified(), "loaded resource should be marked as not modifed");

    test(mr->meshes().size() == 1, "resource should have 1 mesh");
    test(mr->cells().size() == 660, "resource should have 660 cells");
    test(mr->points().size() == 662, "resource should have 662 points");
  }
}
}

int UnitTestExportMesh2DM(int, char** const)
{
  verify_write_empty_resource();
  verify_write_null_resource();
  verify_read_write_valid_resource();

  return 0;
}
