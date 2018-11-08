//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/ImportMesh.h"
#include "smtk/io/ReadMesh.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/mesh/moab/Interface.h"

#include "smtk/mesh/testing/cxx/helpers.h"

namespace
{

//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;

void verify_load_valid_mesh()
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/twoassm_out.h5m";

  smtk::io::ReadMesh read;
  smtk::mesh::ResourcePtr mr = smtk::mesh::Resource::create();
  read(file_path, mr);
  test(mr->isValid(), "resource should be valid");

  std::size_t numMeshes = mr->numberOfMeshes();
  std::cout << "number of meshes in twoassm_out is: " << numMeshes << std::endl;
  test(numMeshes != 0, "dataset once loaded should have more than zero meshes");
  test(numMeshes == 53, "dataset once loaded should have 53 meshes");
}

void verify_load_writeLocation()
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/twoassm_out.h5m";

  smtk::io::ReadMesh read;
  smtk::mesh::InterfacePtr interface = smtk::mesh::moab::make_interface();
  smtk::mesh::ResourcePtr mr = read(file_path, interface);
  test(mr->isValid(), "resource should be valid");

  test(mr->readLocation() == file_path, "resource readLocation is wrong");
  test(mr->writeLocation() == file_path, "resource default writeLocation is wrong");
}

void verify_load_multiple_meshes()
{
  std::string first_file_path(data_root), second_file_path(data_root);
  first_file_path += "/mesh/3d/twoassm_out.h5m";
  second_file_path += "/mesh/3d/64bricks_12ktet.h5m";

  smtk::io::ReadMesh read;
  smtk::mesh::ResourcePtr c1 = smtk::mesh::Resource::create();
  read(first_file_path, c1);
  smtk::mesh::ResourcePtr c2 = smtk::mesh::Resource::create();
  read(second_file_path, c2);

  test(c1->isValid(), "resource should be valid");
  test(c2->isValid(), "resource should be valid");

  //verify the size of twoassm
  {
    std::size_t numMeshes = c1->numberOfMeshes();
    std::cout << "number of meshes in twoassm_out is: " << numMeshes << std::endl;
    test(numMeshes != 0, "dataset once loaded should have more than zero meshes");
    test(numMeshes == 53, "dataset once loaded should have 53 meshes");
  }

  //verify the size of 64bricks
  {
    std::size_t numMeshes = c2->numberOfMeshes();
    std::cout << "number of meshes in 64bricks_12ktet is: " << numMeshes << std::endl;
    test(numMeshes != 0, "dataset once loaded should have more than zero meshes");
    test(numMeshes == 800, "dataset once loaded should have 800 meshes");
  }
}

void verify_load_same_mesh_multiple_times()
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/64bricks_12ktet.h5m";

  smtk::io::ReadMesh read;
  smtk::mesh::ResourcePtr c1 = smtk::mesh::Resource::create();
  read(file_path, c1);
  test(c1->isValid(), "resource should be valid");

  //load the same mesh a second time and confirm that is valid
  smtk::mesh::ResourcePtr c2 = smtk::mesh::Resource::create();
  read(file_path, c2);
  test(c2->isValid(), "resource should be valid");
}

void verify_load_onlyNeumann()
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/64bricks_12ktet.h5m";

  smtk::io::ReadMesh read;
  smtk::mesh::ResourcePtr mr = smtk::mesh::Resource::create();
  read(file_path, mr, smtk::io::mesh::Subset::OnlyNeumann);
  test(mr->isValid(), "resource should be valid");

  std::size_t numMeshes = mr->numberOfMeshes();
  std::cout << "number of neumann meshes in 64bricks_12ktet is: " << numMeshes << std::endl;
  test(numMeshes == 221, "dataset once loaded should have 221 meshes");
}

void verify_load_onlyDirichlet()
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/64bricks_12ktet.h5m";

  smtk::io::ReadMesh read;
  smtk::mesh::ResourcePtr mr = smtk::mesh::Resource::create();
  read(file_path, mr, smtk::io::mesh::Subset::OnlyDirichlet);
  test(mr->isValid(), "resource should be valid");

  std::size_t numMeshes = mr->numberOfMeshes();
  std::cout << "number of dirichlet meshes in 64bricks_12ktet is: " << numMeshes << std::endl;
  test(numMeshes == 221, "dataset once loaded should have 221 meshes");
}
}

int UnitTestLoadMesh(int, char** const)
{
  verify_load_valid_mesh();
  verify_load_writeLocation();

  verify_load_multiple_meshes();

  verify_load_same_mesh_multiple_times();

  verify_load_onlyNeumann();
  verify_load_onlyDirichlet();

  return 0;
}
