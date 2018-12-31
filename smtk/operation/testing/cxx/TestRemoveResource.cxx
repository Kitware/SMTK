//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/common/UUIDGenerator.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Registrar.h"
#include "smtk/operation/operators/RemoveResource.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/DerivedFrom.h"
#include "smtk/resource/Manager.h"

#include "smtk/common/testing/cxx/helpers.h"

namespace
{
class MyResource : public smtk::resource::DerivedFrom<MyResource, smtk::resource::Resource>
{
public:
  smtkTypeMacro(MyResource);
  smtkCreateMacro(MyResource);
  smtkSharedFromThisMacro(smtk::resource::Resource);

  smtk::resource::ComponentPtr find(const smtk::common::UUID&) const override
  {
    return smtk::resource::ComponentPtr();
  }

  std::function<bool(const smtk::resource::ConstComponentPtr&)> queryOperation(
    const std::string&) const override
  {
    return [](const smtk::resource::ConstComponentPtr&) { return true; };
  }

  void visit(smtk::resource::Component::Visitor&) const override {}

protected:
  MyResource()
    : smtk::resource::DerivedFrom<MyResource, smtk::resource::Resource>()
  {
  }
};
}

int TestRemoveResource(int, char** const)
{
  // Create a resource manager
  smtk::resource::Manager::Ptr resourceManager = smtk::resource::Manager::create();

  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  {
    smtk::operation::Registrar::registerTo(operationManager);
  }

  // Register the resource manager to the operation manager (newly created
  // resources will be automatically registered to the resource manager).
  operationManager->registerResourceManager(resourceManager);

  // Register MyResource
  resourceManager->registerResource<MyResource>();

  // Create a new MyResource type
  auto myResource = resourceManager->create<MyResource>();
  smtkTest(resourceManager->resources().size() == 1, "Resource not added to manager.");

  // Create a "Remove Resource" operation
  auto removeResource = operationManager->create<smtk::operation::RemoveResource>();

  // Assign the newly created resource to the operation
  removeResource->parameters()->associate(myResource);

  // Execute the operation
  auto result = removeResource->operate();

  // Test for the operation's success
  smtkTest(result->findInt("outcome")->value() ==
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
    "Remove resource operation failed.");

  // Test that the resource has been removed from the resource manager
  smtkTest(resourceManager->resources().empty() == true, "Resource not removed from manager.");

  return 0;
}
