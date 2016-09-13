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

#include "smtk/mesh/testing/cxx/helpers.h"


namespace
{

//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;

//----------------------------------------------------------------------------
void verify_load_bad_mesh()
{
  std::string file_path(data_root);

  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::mesh::CollectionPtr c = smtk::io::ImportMesh::entireFile(file_path, manager);
  test( !c->isValid(), "collection should be invalid");
  test( c->entity().isNull(), "uuid should be invalid");

  test( (c->readLocation() == file_path), "read location of collection shouldnt be empty");
  test( (c->writeLocation() == file_path), "read location of collection shouldnt be empty");
}

//----------------------------------------------------------------------------
void verify_load_valid_mesh()
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/twoassm_out.h5m";

  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::mesh::CollectionPtr c = smtk::io::ImportMesh::entireFile(file_path, manager);
  test( c->isValid(), "collection should be valid");

  std::size_t numMeshes = c->numberOfMeshes();
  std::cout << "number of meshes in twoassm_out is: " << numMeshes << std::endl;
  test( numMeshes!=0, "dataset once loaded should have more than zero meshes");
  test( numMeshes == 53, "dataset once loaded should have 53 meshes");
}

//----------------------------------------------------------------------------
void verify_load_writeLocation()
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/twoassm_out.h5m";

  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::mesh::CollectionPtr c = smtk::io::ImportMesh::entireFile(file_path, manager);
  test( c->isValid(), "collection should be valid");

  test( c->readLocation() == file_path, "collection readLocation is wrong");
  test( c->writeLocation() == file_path, "collection default writeLocation is wrong");
}

//----------------------------------------------------------------------------
void verify_load_multiple_meshes()
{
  std::string first_file_path(data_root), second_file_path(data_root);
  first_file_path += "/mesh/3d/twoassm_out.h5m";
  second_file_path += "/mesh/3d/64bricks_12ktet.h5m";

  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::mesh::CollectionPtr c1 = smtk::io::ImportMesh::entireFile(first_file_path, manager);
  smtk::mesh::CollectionPtr c2 = smtk::io::ImportMesh::entireFile(second_file_path, manager);

  test( c1->isValid(), "collection should be valid");
  test( c2->isValid(), "collection should be valid");

  //verify the size of twoassm
  {
  std::size_t numMeshes = c1->numberOfMeshes();
  std::cout << "number of meshes in twoassm_out is: " << numMeshes << std::endl;
  test( numMeshes!=0, "dataset once loaded should have more than zero meshes");
  test( numMeshes == 53, "dataset once loaded should have 53 meshes");
  }


  //verify the size of 64bricks
  {
  std::size_t numMeshes = c2->numberOfMeshes();
  std::cout << "number of meshes in 64bricks_12ktet is: " << numMeshes << std::endl;
  test( numMeshes!=0, "dataset once loaded should have more than zero meshes");
  test( numMeshes == 800, "dataset once loaded should have 800 meshes");
  }

  //verify the manager has two collections
  test(manager->numberOfCollections() == 2);
}

//----------------------------------------------------------------------------
void verify_load_same_mesh_multiple_times()
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/64bricks_12ktet.h5m";

  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::mesh::CollectionPtr c1 = smtk::io::ImportMesh::entireFile(file_path, manager);
  test( c1->isValid(), "collection should be valid");
  test(manager->numberOfCollections() == 1);

  //load the same mesh a second time and confirm that is valid
  smtk::mesh::CollectionPtr c2 = smtk::io::ImportMesh::entireFile(file_path, manager);
  test( c2->isValid(), "collection should be valid");
  test(manager->numberOfCollections() == 2);
}

//----------------------------------------------------------------------------
void verify_load_onlyNeumann()
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/64bricks_12ktet.h5m";

  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::mesh::CollectionPtr c = smtk::io::ImportMesh::onlyNeumann(file_path, manager);
  test( c->isValid(), "collection should be valid");

  std::size_t numMeshes = c->numberOfMeshes();
  std::cout << "number of neumann meshes in 64bricks_12ktet is: " << numMeshes << std::endl;
  test( numMeshes == 221, "dataset once loaded should have 221 meshes");

}

//----------------------------------------------------------------------------
void verify_load_onlyDirichlet()
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/64bricks_12ktet.h5m";

  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::mesh::CollectionPtr c = smtk::io::ImportMesh::onlyDirichlet(file_path, manager);
  test( c->isValid(), "collection should be valid");

  std::size_t numMeshes = c->numberOfMeshes();
  std::cout << "number of dirichlet meshes in 64bricks_12ktet is: " << numMeshes << std::endl;
  test( numMeshes == 221, "dataset once loaded should have 221 meshes");
}

//----------------------------------------------------------------------------
void verify_load_onlyDomain()
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/64bricks_12ktet.h5m";

  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::mesh::CollectionPtr c = smtk::io::ImportMesh::onlyDomain(file_path, manager);
  test( c->isValid(), "collection should be valid");

  std::size_t numMeshes = c->numberOfMeshes();
  std::cout << "number of domain meshes in 64bricks_12ktet is: " << numMeshes << std::endl;
  test( numMeshes == 800, "dataset once loaded should have 800 meshes");
}

//----------------------------------------------------------------------------
void verify_load_bad_onlyDomain()
{
  std::cout << "verify_load_bad_onlyDomain" << std::endl;
  std::string file_path(data_root);
  file_path += "/mesh/3d/sixth_hexflatcore.h5m";

  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::mesh::CollectionPtr c = smtk::io::ImportMesh::onlyDomain(file_path, manager);
  //this dataset has no domain sets
  test( !c->isValid(), "collection should be invalid");

}

//----------------------------------------------------------------------------
void verify_load_bad_onlyNeumann()
{
  std::cout << "verify_load_bad_onlyNeumann" << std::endl;
  std::string file_path(data_root);
  file_path += "/mesh/3d/invalid_file.h5m";

  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::mesh::CollectionPtr c = smtk::io::ImportMesh::onlyNeumann(file_path, manager);

  //this dataset has no neumann sets, since it doesn't exist
  //all other tests data sets have neumann sets
  test( !c->isValid(), "collection should be invalid");

}

//----------------------------------------------------------------------------
void verify_load_bad_onlyDirichlet()
{
  std::cout << "verify_load_bad_onlyNeumann" << std::endl;

  std::string file_path(data_root);
  file_path += "/mesh/3d/sixth_hexflatcore.h5m";

  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::mesh::CollectionPtr c = smtk::io::ImportMesh::onlyDirichlet(file_path, manager);

  //this dataset has no dirichlet sets
  test( !c->isValid(), "collection should be invalid");
}

}

//----------------------------------------------------------------------------
int UnitTestLoadMesh(int, char** const)
{
  verify_load_bad_mesh();
  verify_load_valid_mesh();
  verify_load_writeLocation();

  verify_load_multiple_meshes();

  verify_load_same_mesh_multiple_times();

  verify_load_onlyNeumann();
  verify_load_onlyDirichlet();

  verify_load_bad_onlyDomain();
  verify_load_bad_onlyNeumann();
  verify_load_bad_onlyDirichlet();

  return 0;
}
