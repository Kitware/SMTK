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

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"
#include "smtk/mesh/testing/cxx/helpers.h"

namespace
{
//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;

void verify_import_unstructured_grid()
{
  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::mesh::CollectionPtr c = manager->makeCollection();

  std::string file_path(data_root);
  file_path += "/mesh/3d/nickel_superalloy.vtu";

  test (smtk::io::importMesh(file_path, c), "should be able to import unstructured grid");
}

void verify_import_polydata()
{
  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();
  smtk::mesh::CollectionPtr c = manager->makeCollection();

  std::string file_path(data_root);
  file_path += "/scene/BasicScene_12_20_07/PolygonMesh_f8e612a9-876c-4145-9c74-ee6c39f2a157.vtp";

  test (smtk::io::importMesh(file_path, c), "should be able to import polydata");
}
}

//----------------------------------------------------------------------------
int UnitTestMeshIOVTK(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  verify_import_unstructured_grid();

  verify_import_polydata();

  return 0;
}

// This macro ensures the vtk io library is loaded into the executable
smtkComponentInitMacro(smtk_extension_vtk_io_MeshIOVTK)
