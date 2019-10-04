//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/common/UUID.h"

#include "smtk/common/testing/cxx/helpers.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Metadata.h"
#include "smtk/operation/Observer.h"
#include "smtk/operation/Operation.h"
#include "smtk/operation/SpecificationOps.h"

#include <chrono>
#include <future>
#include <thread>

namespace
{
class OperationA : public smtk::operation::Operation
{
public:
  smtkTypeMacro(OperationA);
  smtkCreateMacro(OperationA);
  smtkSharedFromThisMacro(smtk::operation::Operation);

  OperationA() {}
  ~OperationA() override {}

  Result operateInternal() override;

  Specification createSpecification() override;
};

OperationA::Result OperationA::operateInternal()
{
  return this->createResult(Outcome::SUCCEEDED);
}

OperationA::Specification OperationA::createSpecification()
{
  Specification spec = this->createBaseSpecification();
  auto opDef = spec->createDefinition("my operation", "operation");
  auto resultDef = spec->createDefinition("result(my export operation)", "result");
  return spec;
}

class OperationB : public OperationA
{
public:
  smtkTypeMacro(OperationB);
  smtkCreateMacro(OperationB);
  smtkSharedFromThisMacro(smtk::operation::Operation);
};
}

int TestOperationGroup(int, char** const)
{
  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  // Register OperationA to the manager
  operationManager->registerOperation<OperationA>("OperationA");

  // Check that the operation manager has no available groups
  smtkTest(operationManager->availableGroups().empty(),
    "Operation manager should have no available groups.");

  // Create a group
  smtk::operation::Group group("my group", operationManager);

  // Register OperationA to the group and test for success
  bool success = group.registerOperation<OperationA>();
  smtkTest(success, "Failed to register OperationA to my group");

  // Check that the operation manager has one available group
  auto availableGroups = operationManager->availableGroups();
  smtkTest(availableGroups.size() == 1, "Operation manager should have one available group.");

  // Check that the available group is "my group"
  smtkTest(availableGroups.find(group.name()) != availableGroups.end(),
    "Operation manager should have my group.");

  // Check if my group has an operation
  smtkTest(group.operations().size() == 1, "my group should have 1 tag");

  // Check if the operation is accessible by name
  smtkTest(group.contains("OperationA"), "Operation group should access OperationA by name");

  // Check if OperationA has any group values.
  auto values = group.values<OperationA>();
  smtkTest(values.empty(), "OperationA should have no values");

  // Check if the operation is accessible by template parameter
  smtkTest(
    group.contains<OperationA>(), "Operation group should access OperationA by template parameter");

  // Check that OperationB is not in the group
  smtkTest(!group.contains("OperationB"), "Operation group should not access OperationB");

  // Create another instance of my group
  smtk::operation::Group sameGroup(group.name(), operationManager);

  // Try to register OperationB to group "my group" without adding it to the
  // manager (should fail because tags are not inherited).
  success = sameGroup.registerOperation("OperationB");
  smtkTest(!success, "Should not be able to register OperationB to my group");

  // Now register it to the manager and try again.
  operationManager->registerOperation<OperationB>("OperationB");
  success = group.registerOperation("OperationB", { "foo", "bar" });
  smtkTest(success, "Failed to register OperationB to my group");

  // Check if OperationB has any group values.
  values = group.values<OperationB>();
  smtkTest(values.size() == 2, "OperationB should have two values");

  smtkTest(values.find("foo") != values.end(), "OperationB should have value \"foo\"")
    smtkTest(values.find("bar") != values.end(), "OperationB should have value \"bar\"")

    // Check that OperationB is in the group
    smtkTest(group.contains("OperationB"), "Operation group should access OperationB by name");

  // Check that my group now has two tags
  smtkTest(group.operations().size() == 2, "my group should have 2 tags");

  // Attempt to register OperationA again (should fail)
  success = group.registerOperation<OperationA>();
  smtkTest(!success, "OperationA should not be added to my group twice");

  // Unregister OperationA from group "my group" and test for success
  success = group.unregisterOperation<OperationA>();
  smtkTest(success, "OperationA could not be removed from my group");

  // Check that my group is back to one tag
  smtkTest(group.operations().size() == 1, "my group should have 1 tag");

  // Check if the operation is accessible by name (should fail)
  smtkTest(
    !group.contains("OperationA"), "Operation group should not be able to access OperationA");

  // Check if the operation is accessible by template parameter (should fail)
  smtkTest(
    !group.contains<OperationA>(), "Operation group should not be able to access OperationA");

  return 0;
}
