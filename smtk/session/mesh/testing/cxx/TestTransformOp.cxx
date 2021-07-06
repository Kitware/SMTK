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
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/session/mesh/Registrar.h"
#include "smtk/session/mesh/Session.h"
#include "smtk/session/mesh/operators/CreateUniformGrid.h"
#include "smtk/session/mesh/operators/Print.h"
#include "smtk/session/mesh/operators/Transform.h"

#include "smtk/common/testing/cxx/helpers.h"

#include "smtk/geometry/queries/BoundingBox.h"

#include "smtk/model/CellEntity.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Face.h"
#include "smtk/model/Group.h"
#include "smtk/model/Resource.h"

#include "smtk/plugin/Registry.h"

#include "smtk/resource/Manager.h"

#include "smtk/operation/Manager.h"

namespace
{
const double EPSILON = 1.e-10;
}

int TestTransformOp(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  // Create a resource manager
  smtk::resource::Manager::Ptr resourceManager = smtk::resource::Manager::create();

  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  // Register mesh operators to the resource and operation managers
  auto meshRegistry =
    smtk::plugin::addToManagers<smtk::session::mesh::Registrar>(resourceManager, operationManager);

  // Register the resource manager to the operation manager (newly created
  // resources will be automatically registered to the resource manager).
  operationManager->registerResourceManager(resourceManager);

  // Create a "create background domain" operator
  smtk::operation::Operation::Ptr createBackgroundDomainOp =
    operationManager->create<smtk::session::mesh::CreateUniformGrid>();

  if (!createBackgroundDomainOp)
  {
    std::cerr << "Couldn't create \"create uniform grid\" operator" << std::endl;
    return 1;
  }

  createBackgroundDomainOp->parameters()->findString("dimension")->setValue("3");
  createBackgroundDomainOp->parameters()->findDouble("size3d")->setValue(0, 2);

  createBackgroundDomainOp->parameters()->findInt("discretization3d")->setValue(0, 5);
  createBackgroundDomainOp->parameters()->findInt("discretization3d")->setValue(1, 5);
  createBackgroundDomainOp->parameters()->findInt("discretization3d")->setValue(2, 5);

  smtk::operation::Operation::Result createBackgroundDomainOpResult =
    createBackgroundDomainOp->operate();

  if (
    createBackgroundDomainOpResult->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    std::cerr << "\"create uniform grid\" operation failed\n";
    return 1;
  }

  smtk::attribute::ComponentItemPtr componentItem =
    std::dynamic_pointer_cast<smtk::attribute::ComponentItem>(
      createBackgroundDomainOpResult->findComponent("created"));

  smtk::model::Entity::Ptr model =
    std::dynamic_pointer_cast<smtk::model::Entity>(componentItem->value());

  smtk::geometry::BoundingBox& boundingBox =
    model->resource()->queries().get<smtk::geometry::BoundingBox>();

  std::array<double, 6> size = boundingBox(model);

  smtk::operation::Operation::Ptr transformOp =
    operationManager->create<smtk::session::mesh::Transform>();

  transformOp->parameters()->associate(model);

  double scale = 0.5;
  transformOp->parameters()->findDouble("scale")->setValue(0, scale);
  smtk::operation::Operation::Result transformOpResult = transformOp->operate();

  if (
    transformOpResult->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    std::cerr << "\"transform\" operation failed\n";
    return 1;
  }

  std::array<double, 6> transformedSize = boundingBox(model);

  if (fabs(size[1] * scale - transformedSize[1]) >= EPSILON)
  {
    std::cerr << "Transform operation did something unexpected\n";
    return 1;
  }

  return 0;
}
