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
void load_bad_mesh()
{
  std::string file_path(data_root);
  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::common::UUID entity = smtk::io::ImportMesh::intoManager(file_path, manager);
  test( entity.isNull(), "uuid should be invalid");
}

//----------------------------------------------------------------------------
void load_valid_mesh()
{
  std::string file_path(data_root);
  file_path += "/mesh/twoassm_out.h5m";
  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::common::UUID entity = smtk::io::ImportMesh::intoManager(file_path, manager);
  test( !entity.isNull(), "uuid shouldn't be invalid");

  smtk::mesh::Collection c = manager->collection(entity);
  test( c.isValid(), "collection should be valid");

  std::size_t numMeshes = c.numberOfMeshes();
  std::cout << "number of meshes in twoassm_out is: " << numMeshes << std::endl;
  test( numMeshes!=0, "dataset once loaded should have more than zero meshes");
  test( numMeshes == 53, "dataset once loaded should have 53 meshes");

}

}

//----------------------------------------------------------------------------
int UnitTestLoadMesh(int argc, char** argv)
{
  load_bad_mesh();
  load_valid_mesh();

  return 0;
}