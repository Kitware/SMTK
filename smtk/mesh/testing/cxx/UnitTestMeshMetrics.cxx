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
#include "smtk/attribute/MeshItem.h"

#include "smtk/io/ExportMesh.h"
#include "smtk/io/ImportMesh.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"
#include "smtk/mesh/Metrics.h"

#include "smtk/model/Operator.h"

#include "smtk/mesh/testing/cxx/helpers.h"

namespace
{

//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;

void verify_eulerCharacteristic_cube()
{
  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();

  std::string file_path(data_root);
  file_path += "/mesh/3d/cube.exo";

  smtk::mesh::CollectionPtr c = smtk::io::importMesh(file_path, manager);
  test(c->isValid(), "collection should be valid");

  test(eulerCharacteristic(c->meshes().extractShell()) == 2);
  test(eulerCharacteristic(c->meshes()) == 1);
}

void verify_eulerCharacteristic_cubeWithHole()
{
  smtk::mesh::ManagerPtr manager = smtk::mesh::Manager::create();

  std::string file_path(data_root);
  file_path += "/mesh/3d/cube_with_hole.exo";

  smtk::mesh::CollectionPtr c = smtk::io::importMesh(file_path, manager);
  test(c->isValid(), "collection should be valid");

  test(eulerCharacteristic(c->meshes().extractShell()) == 0);
}

void verify_eulerCharacteristicOp()
{
  // Create a model manager
  smtk::model::ManagerPtr manager = smtk::model::Manager::create();

  // Create a new default session
  smtk::model::SessionRef sessRef = manager->createSession("native");

  std::string file_path(data_root);
  file_path += "/mesh/3d/cube.exo";

  smtk::mesh::CollectionPtr c = smtk::io::importMesh(file_path, manager->meshes());
  test(c->isValid(), "collection should be valid");

  // Create an "Euler characteristic" operator
  smtk::model::OperatorPtr eulerCharacteristicOp = sessRef.session()->op("euler characteristic");
  test(eulerCharacteristicOp != nullptr, "No \"euler characteristic\" operator\n");

  // Set the operator's input mesh
  bool valueSet =
    eulerCharacteristicOp->specification()->findMesh("mesh")->setValue(c->meshes().extractShell());

  test(valueSet, "Failed to set mesh value on operator\n");

  // Execute "Euler characteristic" operator...
  smtk::model::OperatorResult eulerCharacteristicOpResult = eulerCharacteristicOp->operate();

  // ...and test the results for success.
  test(eulerCharacteristicOpResult->findInt("outcome")->value() == smtk::model::OPERATION_SUCCEEDED,
    "\"euler characteristic\" operator failed\n");

  // Finally, confirm that the Euler characteristic is correct.
  test(eulerCharacteristicOpResult->findInt("value")->value() == 2);
}
}

int UnitTestMeshMetrics(int, char** const)
{
  verify_eulerCharacteristic_cube();
  verify_eulerCharacteristic_cubeWithHole();
  verify_eulerCharacteristicOp();

  return 0;
}
