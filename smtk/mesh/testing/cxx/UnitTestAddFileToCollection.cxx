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
#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"

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

void verify_cant_append_to_bad_collection()
{
  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();

  //create a bad collection
  std::string file_path(data_root);
  smtk::mesh::CollectionPtr c = smtk::io::readMesh(file_path, manager);
  test(!c->isValid(), "collection should be invalid");

  //append to the bad collection is always a failure
  bool result = smtk::io::readMesh(first_mesh_path(), c);
  test(!result, "read into an invalid collection should fail");
  test(!c->isValid(), "collection should still be invalid");
}

void verify_cant_append_to_null_collection()
{
  smtk::mesh::CollectionPtr null_collection_ptr;
  bool result = smtk::io::readMesh(first_mesh_path(), null_collection_ptr);
  test(!result, "read into a null collection should fail");
}

void verify_append_to_valid_empty_collection()
{
  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::mesh::CollectionPtr collection = manager->makeCollection();

  bool result = smtk::io::readMesh(second_mesh_path(), collection);
  test(result, "read of self into self should work");
  test(collection->isValid(), "collection should be valid");

  std::size_t numMeshes = collection->numberOfMeshes();
  test(numMeshes != 0);
}

void verify_append_self_to_self()
{
  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::io::ReadMesh read;
  smtk::mesh::CollectionPtr c = read(first_mesh_path(), manager);
  test(c->isValid(), "collection should be valid");

  std::size_t origNumMeshes = c->numberOfMeshes();
  test(origNumMeshes != 0);

  //append the mesh back onto itself
  bool result = read(first_mesh_path(), c);
  test(result, "read of self into self should work");
  test(c->isValid(), "collection should be valid");

  //verify the size has doubled
  std::size_t newNumMeshes = c->numberOfMeshes();
  test(newNumMeshes == 2 * origNumMeshes);
}

void verify_append_subsection_of_self()
{
  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::io::ReadMesh read;
  smtk::mesh::CollectionPtr c = read(first_mesh_path(), manager);
  test(c->isValid(), "collection should be valid");
  std::size_t firstNumMesh = c->numberOfMeshes();
  test(firstNumMesh != 0);

  //now load the second mesh into a different manager and get the size
  //this can be used to verify that the append collection has the proper size
  std::size_t secondNumMesh = 0;
  {
    smtk::mesh::CollectionPtr c2 =
      read(first_mesh_path(), manager, smtk::io::mesh::Subset::OnlyNeumann);
    test(c2->isValid(), "second collection load should be valid");
    secondNumMesh = c2->numberOfMeshes();
    test(secondNumMesh != 0);
  }
  //now append a second file and verify the number of meshes is correct
  bool result = read(first_mesh_path(), c, smtk::io::mesh::Subset::OnlyNeumann);
  test(result, "read into a valid collection should work");
  test(c->isValid(), "collection after read should still be valid");

  std::size_t newNumMeshes = c->numberOfMeshes();
  test(newNumMeshes == firstNumMesh + secondNumMesh);
}

void verify_cant_append_mismatched_data()
{
  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::io::ReadMesh read;
  smtk::mesh::CollectionPtr c = read(first_mesh_path(), manager);
  test(c->isValid(), "collection should be valid");
  std::size_t firstNumMesh = c->numberOfMeshes();
  test(firstNumMesh != 0);

  //now append a second file which has different vertices and verify that fails
  bool result = read(second_mesh_path(), c);
  test(!result, "read of a second mesh that has different vertices should fail");
}

void verify_append_dirichlet_to_neumann()
{
  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::io::ReadMesh read;
  smtk::mesh::CollectionPtr c =
    read(first_mesh_path(), manager, smtk::io::mesh::Subset::OnlyNeumann);
  test(c->isValid(), "collection should be valid");
  std::size_t firstNumMesh = c->numberOfMeshes();
  test(firstNumMesh != 0);

  //now load the second mesh into a different manager and get the size
  //this can be used to verify that the append collection has the proper size
  std::size_t secondNumMesh = 0;
  {
    smtk::mesh::CollectionPtr c2 =
      read(first_mesh_path(), manager, smtk::io::mesh::Subset::OnlyDirichlet);
    test(c2->isValid(), "second collection load should be valid");
    secondNumMesh = c2->numberOfMeshes();
    test(secondNumMesh != 0);
  }
  //now append a second file and verify the number of meshes is correct
  bool result = read(first_mesh_path(), c, smtk::io::mesh::Subset::OnlyDirichlet);
  test(result, "import into a valid collection should work");
  test(c->isValid(), "collection after import should still be valid");

  std::size_t newNumMeshes = c->numberOfMeshes();
  test(newNumMeshes == firstNumMesh + secondNumMesh);
}

void verify_append_neumann_to_dirichlet()
{
  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::io::ReadMesh read;
  smtk::mesh::CollectionPtr c =
    read(first_mesh_path(), manager, smtk::io::mesh::Subset::OnlyDirichlet);
  test(c->isValid(), "collection should be valid");
  std::size_t firstNumMesh = c->numberOfMeshes();
  test(firstNumMesh != 0);

  //now load the second mesh into a different manager and get the size
  //this can be used to verify that the append collection has the proper size
  std::size_t secondNumMesh = 0;
  {
    smtk::mesh::CollectionPtr c2 =
      read(first_mesh_path(), manager, smtk::io::mesh::Subset::OnlyNeumann);
    test(c2->isValid(), "second collection load should be valid");
    secondNumMesh = c2->numberOfMeshes();
    test(secondNumMesh != 0);
  }
  //now append a second file and verify the number of meshes is correct
  bool result = read(first_mesh_path(), c, smtk::io::mesh::Subset::OnlyNeumann);
  test(result, "import into a valid collection should work");
  test(c->isValid(), "collection after import should still be valid");

  std::size_t newNumMeshes = c->numberOfMeshes();
  test(newNumMeshes == firstNumMesh + secondNumMesh);
}

void verify_append_domain_to_dirichlet()
{
  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::io::ReadMesh read;
  smtk::mesh::CollectionPtr c =
    read(first_mesh_path(), manager, smtk::io::mesh::Subset::OnlyDirichlet);
  test(c->isValid(), "collection should be valid");
  std::size_t firstNumMesh = c->numberOfMeshes();
  test(firstNumMesh != 0);

  //now load the second mesh into a different manager and get the size
  //this can be used to verify that the append collection has the proper size
  std::size_t secondNumMesh = 0;
  {
    smtk::mesh::CollectionPtr c2 =
      read(first_mesh_path(), manager, smtk::io::mesh::Subset::OnlyDomain);
    test(c2->isValid(), "expected domains in this file");
    secondNumMesh = c2->numberOfMeshes();
    test(secondNumMesh != 0);
  }

  //now append a second file and verify the number of meshes is correct
  bool result = read(first_mesh_path(), c, smtk::io::mesh::Subset::OnlyDomain);
  test(result, "import into a valid collection should work");
  test(c->isValid(), "collection after import should still be valid");

  std::size_t newNumMeshes = c->numberOfMeshes();
  test(newNumMeshes == firstNumMesh + secondNumMesh);
}
}

int UnitTestAddFileToCollection(int, char** const)
{
  //append into a collection that had failed to load previously
  verify_cant_append_to_bad_collection();
  verify_cant_append_to_null_collection();

  //given a valid empty collection append a mesh into the collection
  verify_append_to_valid_empty_collection();

  //append a subsection of a mesh into a collection that had loaded
  //all of the same mesh before
  verify_append_subsection_of_self();

  //append the mesh that was already loaded previously to the same collection
  verify_append_self_to_self();

  //you cant append together two mesh files that have different vertices's
  //since that why cause a geometric inconsistency
  verify_cant_append_mismatched_data();

  //attempt to append Dirichlet to a collection that only has Neumman
  verify_append_dirichlet_to_neumann();

  //attempt to append Neumman set to a collection that only has Dirichlet
  verify_append_neumann_to_dirichlet();

  //attempt to append domain set to a collection that only has Dirichlet
  verify_append_domain_to_dirichlet();

  return 0;
}
