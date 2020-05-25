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
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/session/opencascade/Registrar.h"
#include "smtk/session/opencascade/Resource.h"
#include "smtk/session/opencascade/Session.h"
#include "smtk/session/opencascade/operators/CreateBox.h"

#include "smtk/common/testing/cxx/helpers.h"

#include "smtk/resource/Manager.h"

#include "smtk/operation/Manager.h"

int TestCreateBox(int, char* [])
{
  // Create a resource manager
  smtk::resource::Manager::Ptr resourceManager = smtk::resource::Manager::create();

  // Register opencascade resources to the resource manager
  {
    smtk::session::opencascade::Registrar::registerTo(resourceManager);
  }

  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  // Register opencascade operators to the operation manager
  {
    smtk::session::opencascade::Registrar::registerTo(operationManager);
  }

  // Register the resource manager to the operation manager (newly created
  // resources will be automatically registered to the resource manager).
  operationManager->registerResourceManager(resourceManager);

  // Create a "create box" operator
  smtk::operation::Operation::Ptr createBoxOp =
    operationManager->create<smtk::session::opencascade::CreateBox>();

  if (!createBoxOp)
  {
    std::cerr << "Couldn't create \"create box\" operator" << std::endl;
    return 1;
  }

  createBoxOp->parameters()->findDouble("center")->setValue(0, 0.);
  createBoxOp->parameters()->findDouble("center")->setValue(1, 0.);
  createBoxOp->parameters()->findDouble("center")->setValue(2, 0.);

  createBoxOp->parameters()->findDouble("size")->setValue(0, 1.);
  createBoxOp->parameters()->findDouble("size")->setValue(1, 1.);
  createBoxOp->parameters()->findDouble("size")->setValue(2, 1.);

  smtk::operation::Operation::Result createBoxOpResult = createBoxOp->operate();

  if (createBoxOpResult->findInt("outcome")->value() !=
    static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
  {
    std::cerr << "\"create box\" operator failed\n";
    return 1;
  }

  smtk::attribute::ResourceItemPtr resourceItem = createBoxOpResult->findResource("resource");

  smtk::session::opencascade::Resource::Ptr resource =
    std::dynamic_pointer_cast<smtk::session::opencascade::Resource>(resourceItem->value());

  std::cout << "there are " << resource->nodes().size() << " nodes" << std::endl;

  return 0;
}
