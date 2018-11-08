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
#include "smtk/io/ReadMesh.h"
#include "smtk/io/WriteMesh.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/mesh/moab/Interface.h"

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
  std::string file_path(data_root);
  file_path += "/mesh/output.h5m";

  std::string write_path(write_root);
  write_path += "/" + smtk::common::UUID::random().toString() + ".h5m";

  smtk::mesh::ResourcePtr mr = smtk::mesh::Resource::create();
  test(mr->isValid(), "empty resource is empty");

  smtk::io::WriteMesh write;
  bool result = write(write_path, mr);

  //before we verify if the write was good, first remove the output file
  cleanup(write_path);
  test(result == true, "Wrote empty resource to disk");
}

void verify_write_null_resource()
{
  std::string write_path(write_root);
  write_path += "/" + smtk::common::UUID::random().toString() + ".h5m";

  //use a null resource ptr
  smtk::mesh::ResourcePtr mr;

  smtk::io::WriteMesh write;
  bool result = write(write_path, mr);

  //before we verify if the write was good, first remove the output file
  cleanup(write_path);

  test(result == false, "Can't save null resource to disk");
}

void verify_write_valid_resource_hdf5()
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/twoassm_out.h5m";

  std::string write_path(write_root);
  write_path += "/" + smtk::common::UUID::random().toString() + ".h5m";

  smtk::io::ReadMesh read;
  smtk::mesh::ResourcePtr mr = smtk::mesh::Resource::create();
  read(file_path, mr);
  test(mr->isValid(), "resource should be valid");
  test(!mr->isModified(), "resource shouldn't be marked as modified");

  //write out the mesh.
  smtk::io::WriteMesh write;
  bool result = write(write_path, mr);
  if (!result)
  {
    cleanup(write_path);
    test(result == true, "failed to properly write out a valid hdf5 resource");
  }

  //reload the written file and verify the number of meshes are the same as the
  //input mesh
  smtk::mesh::ResourcePtr mr1 = smtk::mesh::Resource::create();
  read(write_path, mr1);
  test(!mr1->isModified(), "resource shouldn't be marked as modified");

  //remove the file from disk
  cleanup(write_path);

  //verify the meshes
  test(mr1->isValid(), "resource should be valid");
  test(mr1->name() == mr->name());
  test(mr1->numberOfMeshes() == mr->numberOfMeshes());
  test(mr1->types() == mr->types());
}

void verify_write_valid_resource_exodus()
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/twoassm_out.h5m";

  std::string write_path(write_root);
  write_path += "/" + smtk::common::UUID::random().toString() + ".exo";

  smtk::io::ReadMesh read;
  smtk::mesh::ResourcePtr mr = smtk::mesh::Resource::create();
  read(file_path, mr);
  test(mr->isValid(), "resource should be valid");

  //write out the mesh.
  smtk::io::WriteMesh write;
  bool result = write(write_path, mr);
  if (!result)
  {
    cleanup(write_path);
    test(result == true, "failed to properly write out a valid exodus resource");
  }

  //When exporting as an exodus file we only write out the volume elements
  //so that is what we should verify are the same
  smtk::mesh::ResourcePtr mr1 = smtk::mesh::Resource::create();
  read(write_path, mr1);

  //remove the file from disk
  cleanup(write_path);

  //verify the meshes
  test(mr1->isValid(), "resource should be valid");
  test(mr1->name() == mr->name());

  test(mr1->meshes(smtk::mesh::Dims3).size() == mr->meshes(smtk::mesh::Dims3).size());
  test(mr1->cells(smtk::mesh::Dims3).size() == mr->cells(smtk::mesh::Dims3).size());
}

void verify_write_valid_resource_using_write_path()
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/twoassm_out.h5m";

  std::string write_path(write_root);
  write_path += "/" + smtk::common::UUID::random().toString() + ".h5m";

  smtk::mesh::InterfacePtr interface = smtk::mesh::moab::make_interface();
  smtk::io::ReadMesh read;
  smtk::mesh::ResourcePtr mr = read(file_path, interface);
  test(mr->isValid(), "resource should be valid");

  test(mr->readLocation() == file_path, "readLocation should match file_path");

  mr->writeLocation(write_path);
  test(mr->writeLocation() == write_path, "writeLocation should match write_path");
  test(!mr->isModified(), "changing write path shouldn't change modified flag");

  //write out the mesh.
  smtk::io::WriteMesh write;
  bool result = write(mr);
  if (!result)
  {
    cleanup(write_path);
    test(result == true, "failed to properly write out a valid hdf5 resource");
  }

  //reload the written file and verify the number of meshes are the same as the
  //input mesh
  smtk::mesh::ResourcePtr mr1 = smtk::mesh::Resource::create();
  read(write_path, mr1);

  //remove the file from disk
  cleanup(write_path);

  //verify the meshes
  test(mr1->isValid(), "resource should be valid");
  test(mr1->name() == mr->name());
  test(mr1->numberOfMeshes() == mr->numberOfMeshes());
  test(mr1->types() == mr->types());
}

void verify_write_valid_resource_using_functions()
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/twoassm_out.h5m";

  std::string write_path(write_root);
  write_path += "/" + smtk::common::UUID::random().toString() + ".h5m";

  smtk::mesh::InterfacePtr interface = smtk::mesh::moab::make_interface();
  smtk::mesh::ResourcePtr mr = smtk::io::readMesh(file_path, interface);
  test(mr->isValid(), "resource should be valid");

  test(mr->readLocation() == file_path, "readLocation should match file_path");

  mr->writeLocation(write_path);
  test(mr->writeLocation() == write_path, "writeLocation should match write_path");
  test(!mr->isModified(), "changing write path shouldn't change modified flag");

  //write out the mesh.
  bool result = smtk::io::writeMesh(mr);
  if (!result)
  {
    cleanup(write_path);
    test(result == true, "failed to properly write out a valid hdf5 resource");
  }

  //reload the written file and verify the number of meshes are the same as the
  //input mesh
  smtk::mesh::ResourcePtr mr1 = smtk::mesh::Resource::create();
  smtk::io::readMesh(write_path, mr1);

  //remove the file from disk
  cleanup(write_path);

  //verify the meshes
  test(mr1->isValid(), "resource should be valid");
  test(mr1->name() == mr->name());
  test(mr1->numberOfMeshes() == mr->numberOfMeshes());
  test(mr1->types() == mr->types());
}

void verify_write_onlyDomain()
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/64bricks_12ktet.h5m";

  std::string write_path(write_root);
  write_path += "/" + smtk::common::UUID::random().toString() + ".h5m";

  smtk::io::ReadMesh read;
  smtk::mesh::ResourcePtr mr = smtk::mesh::Resource::create();
  read(file_path, mr);
  test(mr->isValid(), "resource should be valid");

  //write out the mesh.
  smtk::io::WriteMesh write;
  bool result = write(write_path, mr, smtk::io::mesh::Subset::OnlyDomain);
  if (!result)
  {
    cleanup(write_path);
    test(result == true, "failed to properly write out only Material");
  }

  //reload the written file and verify the number of meshes are the same as the
  //input mesh
  smtk::mesh::ResourcePtr mr1 = smtk::mesh::Resource::create();
  read(write_path, mr1);
  smtk::mesh::ResourcePtr mr2 = smtk::mesh::Resource::create();
  read(file_path, mr2, smtk::io::mesh::Subset::OnlyDomain);

  // remove the file from disk
  cleanup(write_path);

  // //verify the meshes
  test(mr1->isValid(), "resource should be valid");
  test(mr1->name() == mr2->name());
  //need to dig into why we are getting a meshset on extraction that is not
  //part of the set
  test(mr1->cells().size() == mr2->cells().size());
  test(mr1->pointConnectivity().size() == mr2->pointConnectivity().size());
  test(mr1->types() == mr2->types());
}

void verify_write_onlyNeumann()
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/64bricks_12ktet.h5m";

  std::string write_path(write_root);
  write_path += "/" + smtk::common::UUID::random().toString() + ".h5m";

  smtk::io::ReadMesh read;
  smtk::mesh::ResourcePtr mr = smtk::mesh::Resource::create();
  read(file_path, mr);
  test(mr->isValid(), "resource should be valid");

  //write out the mesh.
  smtk::io::WriteMesh write;
  bool result = write(write_path, mr, smtk::io::mesh::Subset::OnlyNeumann);
  if (!result)
  {
    cleanup(write_path);
    test(result == true, "failed to properly write out only Neumann");
  }

  //reload the written file and verify the number of meshes are the same as the
  //input mesh
  smtk::mesh::ResourcePtr mr1 = smtk::mesh::Resource::create();
  read(write_path, mr1);
  smtk::mesh::ResourcePtr mr2 = smtk::mesh::Resource::create();
  read(file_path, mr2, smtk::io::mesh::Subset::OnlyNeumann);

  // remove the file from disk
  cleanup(write_path);

  // //verify the meshes
  test(mr1->isValid(), "resource should be valid");
  test(mr1->name() == mr2->name());
  //need to dig into why we are getting a meshset on extraction that is not
  //part of the set
  test(mr1->numberOfMeshes() + 1 == mr2->numberOfMeshes());
  test(mr1->cells().size() == mr2->cells().size());
  test(mr1->pointConnectivity().size() == mr2->pointConnectivity().size());
  test(mr1->types() == mr2->types());
}

void verify_write_onlyDirichlet()
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/64bricks_12ktet.h5m";

  std::string write_path(write_root);
  write_path += "/" + smtk::common::UUID::random().toString() + ".h5m";

  smtk::io::ReadMesh read;
  smtk::mesh::ResourcePtr mr = smtk::mesh::Resource::create();
  read(file_path, mr);
  test(mr->isValid(), "resource should be valid");

  //write out the mesh.
  smtk::io::WriteMesh write;
  bool result = write(write_path, mr, smtk::io::mesh::Subset::OnlyDirichlet);
  if (!result)
  {
    cleanup(write_path);
    test(result == true, "failed to properly write out only Dirichlet");
  }

  //reload the written file and verify the number of meshes are the same as the
  //input mesh
  smtk::mesh::ResourcePtr mr1 = smtk::mesh::Resource::create();
  read(write_path, mr1);
  smtk::mesh::ResourcePtr mr2 = smtk::mesh::Resource::create();
  read(file_path, mr2, smtk::io::mesh::Subset::OnlyDirichlet);

  //remove the file from disk
  cleanup(write_path);

  //verify the meshes
  test(mr1->isValid(), "resource should be valid");
  test(mr1->name() == mr2->name());
  //need to dig into why we are getting a meshset on extraction that is not
  //part of the set
  test(mr1->numberOfMeshes() + 1 == mr2->numberOfMeshes());
  test(mr1->cells().size() == mr2->cells().size());
  test(mr1->pointConnectivity().size() == mr2->pointConnectivity().size());
  test(mr1->types() == mr2->types());
}

void verify_write_clears_modified_flag()
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/64bricks_12ktet.h5m";

  std::string write_path(write_root);
  write_path += "/" + smtk::common::UUID::random().toString() + ".h5m";

  smtk::io::ReadMesh read;
  smtk::mesh::ResourcePtr mr = smtk::mesh::Resource::create();
  read(file_path, mr);
  test(mr->isValid(), "resource should be valid");
  test(!mr->isModified(), "resource loaded from disk shouldn't be modified");

  smtk::mesh::MeshSet meshes3D = mr->meshes(smtk::mesh::Dims3);
  smtk::mesh::MeshSet shell = meshes3D.extractShell();
  test(mr->isModified(), "extracting the shell should mark the resource as modified");

  //write out the mesh.
  smtk::io::WriteMesh write;
  bool result = write(write_path, mr);
  if (!result)
  {
    cleanup(write_path);
    test(result == true, "failed to properly write out the mesh");
  }

  test(!mr->isModified(), "after a write the resource should not be modified");

  //now remove the shell, and verify the mesh is again marked as modified
  mr->removeMeshes(shell);
  test(mr->isModified(), "after mesh removal the resource should be modified");

  //write out the mesh again
  result = write(write_path, mr);
  if (!result)
  {
    cleanup(write_path);
    test(result == true, "failed to properly write out the mesh");
  }
  test(!mr->isModified(), "after a write the resource should not be modified");

  //remove the file from disk
  cleanup(write_path);
}
}

int UnitTestWriteMesh(int, char** const)
{
  verify_write_empty_resource();
  verify_write_null_resource();

  verify_write_valid_resource_hdf5();
  verify_write_valid_resource_exodus();
  verify_write_valid_resource_using_write_path();
  verify_write_valid_resource_using_functions();

  verify_write_onlyDomain();
  verify_write_onlyNeumann();
  verify_write_onlyDirichlet();

  verify_write_clears_modified_flag();

  return 0;
}
