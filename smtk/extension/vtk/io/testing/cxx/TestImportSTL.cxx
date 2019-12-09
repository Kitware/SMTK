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

#include "smtk/mesh/core/Resource.h"
#include "smtk/mesh/testing/cxx/helpers.h"

namespace
{
//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;

void verify_import_stl()
{
  smtk::mesh::ResourcePtr c = smtk::mesh::Resource::create();

  std::string file_path(data_root);
  file_path += "/model/3d/stl/sphere.stl";

  std::cout << "file_path: " << file_path << std::endl;
  test(smtk::io::importMesh(file_path, c), "should be able to import stl file");
  test(!c->meshes().is_empty(), "aggregate meshset should be nonempty");
}
}

int TestImportSTL(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  verify_import_stl();

  return 0;
}

// This macro ensures the vtk io library is loaded into the executable
smtkComponentInitMacro(smtk_extension_vtk_io_mesh_MeshIOVTK)
