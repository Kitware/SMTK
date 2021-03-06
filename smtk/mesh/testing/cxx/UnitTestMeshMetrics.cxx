//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"

#include "smtk/io/ExportMesh.h"
#include "smtk/io/ImportMesh.h"

#include "smtk/mesh/core/Resource.h"

#include "smtk/mesh/utility/Metrics.h"

#include "smtk/mesh/testing/cxx/helpers.h"

namespace
{

//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;

void verify_eulerCharacteristic_cube()
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/cube.exo";

  smtk::mesh::ResourcePtr mr = smtk::mesh::Resource::create();
  smtk::io::importMesh(file_path, mr);
  test(mr->isValid(), "resource should be valid");

  test(smtk::mesh::utility::eulerCharacteristic(mr->meshes().extractShell()) == 2);
  test(smtk::mesh::utility::eulerCharacteristic(mr->meshes()) == 1);
}

void verify_eulerCharacteristic_cubeWithHole()
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/cube_with_hole.exo";

  smtk::mesh::ResourcePtr mr = smtk::mesh::Resource::create();
  smtk::io::importMesh(file_path, mr);
  test(mr->isValid(), "resource should be valid");

  test(smtk::mesh::utility::eulerCharacteristic(mr->meshes().extractShell()) == 0);
}
} // namespace

int UnitTestMeshMetrics(int /*unused*/, char** const /*unused*/)
{
  verify_eulerCharacteristic_cube();
  verify_eulerCharacteristic_cubeWithHole();

  return 0;
}
