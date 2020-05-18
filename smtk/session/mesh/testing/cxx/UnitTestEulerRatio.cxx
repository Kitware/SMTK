//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/mesh/Resource.h"
#include "smtk/session/mesh/operators/EulerCharacteristicRatio.h"
#include "smtk/session/mesh/operators/Import.h"

#include "smtk/common/UUID.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"

#include <cmath>

#include "smtk/operation/Manager.h"

using namespace smtk::model;

namespace
{
std::string dataRoot = SMTK_DATA_DIR;
}

int UnitTestEulerRatio(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  // Create a resource manager
  smtk::resource::Manager::Ptr resourceManager = smtk::resource::Manager::create();

  // Register the mesh resource to the resource manager
  {
    resourceManager->registerResource<smtk::session::mesh::Resource>();
  }

  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  // Register import and euler characteristic ratio operators to the operation manager
  {
    operationManager->registerOperation<smtk::session::mesh::Import>("smtk::session::mesh::Import");
    operationManager->registerOperation<smtk::session::mesh::EulerCharacteristicRatio>(
      "smtk::session::mesh::EulerCharacteristicRatio");
  }

  // Register the resource manager to the operation manager (newly created
  // resources will be automatically registered to the resource manager).
  operationManager->registerResourceManager(resourceManager);

  smtk::model::Entity::Ptr model;

  {
    // Create an import operator
    smtk::session::mesh::Import::Ptr importOp =
      operationManager->create<smtk::session::mesh::Import>();
    if (!importOp)
    {
      std::cerr << "No import operator\n";
      return 1;
    }

    // Set the file path
    std::string importFilePath(dataRoot);
    importFilePath += "/mesh/3d/cube.exo";
    importOp->parameters()->findFile("filename")->setValue(importFilePath);

    // Execute the operation
    smtk::operation::Operation::Result importOpResult = importOp->operate();

    // Retrieve the resulting model
    smtk::attribute::ComponentItemPtr componentItem =
      std::dynamic_pointer_cast<smtk::attribute::ComponentItem>(
        importOpResult->findComponent("model"));

    // Access the generated model
    model = std::dynamic_pointer_cast<smtk::model::Entity>(componentItem->value());

    // Test for success
    if (
      importOpResult->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      std::cerr << "Import operator failed\n";
      return 1;
    }
  }

  {
    smtk::operation::Operation::Ptr eulerOp =
      operationManager->create<smtk::session::mesh::EulerCharacteristicRatio>();

    if (!eulerOp)
    {
      std::cerr << "No \"euler characteristic ratio\" operator\n";
      return 1;
    }

    eulerOp->parameters()->associateEntity(model->referenceAs<smtk::model::Model>());

    smtk::operation::Operation::Result eulerOpResult = eulerOp->operate();
    if (
      eulerOpResult->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      std::cerr << "\"Euler characteristic ratio\" operator failed\n";
      return 1;
    }

    if (std::abs(eulerOpResult->findDouble("value")->value() - 2.) > 1.e-10)
    {
      std::cerr << "Unexpected Euler ratio\n";
      return 1;
    }
  }

  return 0;
}
