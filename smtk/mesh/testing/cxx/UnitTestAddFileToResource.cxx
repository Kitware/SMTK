//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/ReadMesh.h"

#include "smtk/mesh/core/Resource.h"

#include "smtk/mesh/testing/cxx/helpers.h"

namespace
{

//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;

std::string first_mesh_path()
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/64bricks_12ktet.h5m";
  return file_path;
}

std::string second_mesh_path()
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/twoassm_out.h5m";
  return file_path;
}

void verify_cant_append_to_null_resource()
{
  smtk::mesh::ResourcePtr null_resource_ptr;
  bool result = smtk::io::readMesh(first_mesh_path(), null_resource_ptr);
  test(!result, "read into a null resource should fail");
}

void verify_append_to_valid_empty_resource()
{
  smtk::mesh::ResourcePtr resource = smtk::mesh::Resource::create();

  bool result = smtk::io::readMesh(second_mesh_path(), resource);
  test(result, "read of self into self should work");
  test(resource->isValid(), "resource should be valid");

  std::size_t numMeshes = resource->numberOfMeshes();
  test(numMeshes != 0);
}

void verify_append_self_to_self()
{
  smtk::io::ReadMesh read;
  smtk::mesh::ResourcePtr mr = smtk::mesh::Resource::create();
  read(first_mesh_path(), mr);
  test(mr->isValid(), "resource should be valid");

  std::size_t origNumMeshes = mr->numberOfMeshes();
  test(origNumMeshes != 0);

  //append the mesh back onto itself
  bool result = read(first_mesh_path(), mr);
  test(result, "read of self into self should work");
  test(mr->isValid(), "resource should be valid");

  //verify the size has doubled
  std::size_t newNumMeshes = mr->numberOfMeshes();
  test(newNumMeshes == 2 * origNumMeshes);
}

void verify_append_subsection_of_self()
{
  smtk::io::ReadMesh read;
  smtk::mesh::ResourcePtr mr = smtk::mesh::Resource::create();
  read(first_mesh_path(), mr);
  test(mr->isValid(), "resource should be valid");
  std::size_t firstNumMesh = mr->numberOfMeshes();
  test(firstNumMesh != 0);

  //now load the second mesh into a different manager and get the size
  //this can be used to verify that the append resource has the proper size
  std::size_t secondNumMesh = 0;
  {
    smtk::mesh::ResourcePtr mr1 =
      read(first_mesh_path(), mr->interface(), smtk::io::mesh::Subset::OnlyNeumann);
    test(mr1->isValid(), "second resource load should be valid");
    secondNumMesh = mr1->numberOfMeshes();
    test(secondNumMesh != 0);
  }
  //now append a second file and verify the number of meshes is correct
  bool result = read(first_mesh_path(), mr, smtk::io::mesh::Subset::OnlyNeumann);
  test(result, "read into a valid resource should work");
  test(mr->isValid(), "resource after read should still be valid");

  std::size_t newNumMeshes = mr->numberOfMeshes();
  test(newNumMeshes == firstNumMesh + secondNumMesh);
}

void verify_cant_append_mismatched_data()
{
  smtk::io::ReadMesh read;
  smtk::mesh::ResourcePtr mr = smtk::mesh::Resource::create();
  read(first_mesh_path(), mr);
  test(mr->isValid(), "resource should be valid");
  std::size_t firstNumMesh = mr->numberOfMeshes();
  test(firstNumMesh != 0);

  //now append a second file which has different vertices and verify that fails
  bool result = read(second_mesh_path(), mr);
  test(!result, "read of a second mesh that has different vertices should fail");
}

void verify_append_dirichlet_to_neumann()
{
  smtk::io::ReadMesh read;
  smtk::mesh::ResourcePtr mr = smtk::mesh::Resource::create();
  read(first_mesh_path(), mr, smtk::io::mesh::Subset::OnlyNeumann);
  test(mr->isValid(), "resource should be valid");
  std::size_t firstNumMesh = mr->numberOfMeshes();
  test(firstNumMesh != 0);

  //now load the second mesh into a different manager and get the size
  //this can be used to verify that the append resource has the proper size
  std::size_t secondNumMesh = 0;
  {
    smtk::mesh::ResourcePtr mr1 =
      read(first_mesh_path(), mr->interface(), smtk::io::mesh::Subset::OnlyDirichlet);
    test(mr1->isValid(), "second resource load should be valid");
    secondNumMesh = mr1->numberOfMeshes();
    test(secondNumMesh != 0);
  }
  //now append a second file and verify the number of meshes is correct
  bool result = read(first_mesh_path(), mr, smtk::io::mesh::Subset::OnlyDirichlet);
  test(result, "import into a valid resource should work");
  test(mr->isValid(), "resource after import should still be valid");

  std::size_t newNumMeshes = mr->numberOfMeshes();
  test(newNumMeshes == firstNumMesh + secondNumMesh);
}

void verify_append_neumann_to_dirichlet()
{
  smtk::io::ReadMesh read;
  smtk::mesh::ResourcePtr mr = smtk::mesh::Resource::create();
  read(first_mesh_path(), mr, smtk::io::mesh::Subset::OnlyDirichlet);
  test(mr->isValid(), "resource should be valid");
  std::size_t firstNumMesh = mr->numberOfMeshes();
  test(firstNumMesh != 0);

  //now load the second mesh into a different manager and get the size
  //this can be used to verify that the append resource has the proper size
  std::size_t secondNumMesh = 0;
  {
    smtk::mesh::ResourcePtr mr1 =
      read(first_mesh_path(), mr->interface(), smtk::io::mesh::Subset::OnlyNeumann);
    test(mr1->isValid(), "second resource load should be valid");
    secondNumMesh = mr1->numberOfMeshes();
    test(secondNumMesh != 0);
  }
  //now append a second file and verify the number of meshes is correct
  bool result = read(first_mesh_path(), mr, smtk::io::mesh::Subset::OnlyNeumann);
  test(result, "import into a valid resource should work");
  test(mr->isValid(), "resource after import should still be valid");

  std::size_t newNumMeshes = mr->numberOfMeshes();
  test(newNumMeshes == firstNumMesh + secondNumMesh);
}

void verify_append_domain_to_dirichlet()
{
  smtk::io::ReadMesh read;
  smtk::mesh::ResourcePtr mr = smtk::mesh::Resource::create();
  read(first_mesh_path(), mr, smtk::io::mesh::Subset::OnlyDirichlet);
  test(mr->isValid(), "resource should be valid");
  std::size_t firstNumMesh = mr->numberOfMeshes();
  test(firstNumMesh != 0);

  //now load the second mesh into a different manager and get the size
  //this can be used to verify that the append resource has the proper size
  std::size_t secondNumMesh = 0;
  {
    smtk::mesh::ResourcePtr mr1 =
      read(first_mesh_path(), mr->interface(), smtk::io::mesh::Subset::OnlyDomain);
    test(mr1->isValid(), "expected domains in this file");
    secondNumMesh = mr1->numberOfMeshes();
    test(secondNumMesh != 0);
  }

  //now append a second file and verify the number of meshes is correct
  bool result = read(first_mesh_path(), mr, smtk::io::mesh::Subset::OnlyDomain);
  test(result, "import into a valid resource should work");
  test(mr->isValid(), "resource after import should still be valid");

  std::size_t newNumMeshes = mr->numberOfMeshes();
  test(newNumMeshes == firstNumMesh + secondNumMesh);
}
}

int UnitTestAddFileToResource(int, char** const)
{
  //append into a resource that had failed to load previously
  verify_cant_append_to_null_resource();

  //given a valid empty resource append a mesh into the resource
  verify_append_to_valid_empty_resource();

  //append a subsection of a mesh into a resource that had loaded
  //all of the same mesh before
  verify_append_subsection_of_self();

  //append the mesh that was already loaded previously to the same resource
  verify_append_self_to_self();

  //you cant append together two mesh files that have different vertices's
  //since that why cause a geometrimr inconsistency
  verify_cant_append_mismatched_data();

  //attempt to append Dirichlet to a resource that only has Neumman
  verify_append_dirichlet_to_neumann();

  //attempt to append Neumman set to a resource that only has Dirichlet
  verify_append_neumann_to_dirichlet();

  //attempt to append domain set to a resource that only has Dirichlet
  verify_append_domain_to_dirichlet();

  return 0;
}
