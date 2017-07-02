//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/ExportMesh.h"
#include "smtk/io/ImportMesh.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"
#include "smtk/mesh/Metrics.h"

#include "smtk/mesh/testing/cxx/helpers.h"

namespace
{

//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;

void verify_eulerCharacteristic_cube()
{
  smtk::mesh::ManagerPtr mngr = smtk::mesh::Manager::create();

  std::string file_path(data_root);
  file_path += "/mesh/3d/cube.exo";

  smtk::mesh::CollectionPtr c = smtk::io::importMesh(file_path, mngr);
  test(c->isValid(), "collection should be valid");

  test(eulerCharacteristic(c->meshes().extractShell()) == 2);
}

void verify_eulerCharacteristic_cubeWithHole()
{
  smtk::mesh::ManagerPtr mngr = smtk::mesh::Manager::create();

  std::string file_path(data_root);
  file_path += "/mesh/3d/cube_with_hole.exo";

  smtk::mesh::CollectionPtr c = smtk::io::importMesh(file_path, mngr);
  test(c->isValid(), "collection should be valid");

  test(eulerCharacteristic(c->meshes().extractShell()) == 0);
}
}

int UnitTestMeshMetrics(int, char** const)
{
  verify_eulerCharacteristic_cube();
  verify_eulerCharacteristic_cubeWithHole();

  return 0;
}
