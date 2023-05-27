//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/operation/Manager.h"
#include "smtk/operation/Metadata.h"
#include "smtk/operation/Observer.h"
#include "smtk/operation/Operation.h"
#include "smtk/operation/XMLOperation.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"

#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"

#include "smtk/resource/DerivedFrom.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Metadata.h"
#include "smtk/resource/Resource.h"

#include "smtk/common/testing/cxx/helpers.h"

#include "smtk/AutoInit.h"

#include <iostream>

namespace
{
class ResourceA : public smtk::resource::DerivedFrom<ResourceA, smtk::resource::Resource>
{
public:
  smtkTypeMacro(ResourceA);
  smtkCreateMacro(ResourceA);
  smtkSharedFromThisMacro(smtk::resource::PersistentObject);

  smtk::resource::ComponentPtr find(const smtk::common::UUID& /*compId*/) const override
  {
    return smtk::resource::ComponentPtr();
  }

  std::function<bool(const smtk::resource::Component&)> queryOperation(
    const std::string& /*unused*/) const override
  {
    return [](const smtk::resource::Component& /*unused*/) { return true; };
  }

  void visit(smtk::resource::Component::Visitor& /*v*/) const override {}

protected:
  ResourceA() = default;
};

class ResourceB : public smtk::resource::DerivedFrom<ResourceB, ResourceA>
{
public:
  smtkTypeMacro(ResourceB);
  smtkSharedPtrCreateMacro(smtk::resource::PersistentObject);

protected:
  ResourceB() = default;
};

class ResourceX : public smtk::resource::DerivedFrom<ResourceX, smtk::resource::Resource>
{
public:
  smtkTypeMacro(ResourceX);
  smtkSharedPtrCreateMacro(smtk::resource::PersistentObject);

  smtk::resource::ComponentPtr find(const smtk::common::UUID& /*compId*/) const override
  {
    return smtk::resource::ComponentPtr();
  }

  std::function<bool(const smtk::resource::Component&)> queryOperation(
    const std::string& /*unused*/) const override
  {
    return [](const smtk::resource::Component& /*unused*/) { return true; };
  }

  void visit(smtk::resource::Component::Visitor& /*v*/) const override {}

protected:
  ResourceX() = default;
};

class MyComponent : public smtk::resource::Component
{
public:
  smtkTypeMacro(MyComponent);
  smtkCreateMacro(MyComponent);
  smtkSharedFromThisMacro(smtk::resource::Component);

  const smtk::common::UUID& id() const override { return myId; }
  bool setId(const smtk::common::UUID& anId) override
  {
    myId = anId;
    return true;
  }

  const smtk::resource::ResourcePtr resource() const override { return myResource; }
  void setResource(smtk::resource::Resource::Ptr& r) { myResource = r; }

private:
  smtk::resource::Resource::Ptr myResource;
  smtk::common::UUID myId;
};

class OperationA : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(OperationA);
  smtkCreateMacro(OperationA);
  smtkSharedFromThisMacro(smtk::operation::Operation);

  OperationA() = default;
  ~OperationA() override = default;

  Result operateInternal() override { return this->createResult(Outcome::SUCCEEDED); }

  const char* xmlDescription() const override;
};

const char operationA_xml[] =
  "<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
  "<SMTK_AttributeResource Version=\"3\">"
  "  <Definitions>"
  "    <AttDef Type=\"operation\" Label=\"operation\" Abstract=\"True\">"
  "      <ItemDefinitions>"
  "        <Int Name=\"debug level\" Optional=\"True\">"
  "          <DefaultValue>0</DefaultValue>"
  "        </Int>"
  "      </ItemDefinitions>"
  "    </AttDef>"
  "    <AttDef Type=\"result\" Abstract=\"True\">"
  "      <ItemDefinitions>"
  "        <Int Name=\"outcome\" Label=\"outcome\" Optional=\"False\" NumberOfRequiredValues=\"1\">"
  "        </Int>"
  "        <String Name=\"log\" Optional=\"True\" NumberOfRequiredValues=\"0\" Extensible=\"True\">"
  "        </String>"
  "      </ItemDefinitions>"
  "    </AttDef>"
  "    <AttDef Type=\"OperationA\" Label=\"A Test Operation\" BaseType=\"operation\">"
  "      <ItemDefinitions>"
  "        <Component Name=\"my component\" Optional=\"False\" NumberOfRequiredValues=\"1\">"
  "          <Accepts>"
  "            <Resource Name=\"ResourceA\" Filter=\"\"/>"
  "          </Accepts>"
  "        </Component>"
  "      </ItemDefinitions>"
  "    </AttDef>"
  "    <AttDef Type=\"result(OperationA)\" BaseType=\"result\">"
  "    </AttDef>"
  "  </Definitions>"
  "</SMTK_AttributeResource>";

const char* OperationA::xmlDescription() const
{
  return operationA_xml;
}

class OperationB : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(OperationB);
  smtkCreateMacro(OperationB);
  smtkSharedFromThisMacro(smtk::operation::Operation);

  OperationB() = default;
  ~OperationB() override = default;

  Result operateInternal() override { return this->createResult(Outcome::SUCCEEDED); }

  const char* xmlDescription() const override;
};

const char operationB_xml[] =
  "<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
  "<SMTK_AttributeResource Version=\"3\">"
  "  <Definitions>"
  "    <AttDef Type=\"operation\" Label=\"operation\" Abstract=\"True\">"
  "      <ItemDefinitions>"
  "        <Int Name=\"debug level\" Optional=\"True\">"
  "          <DefaultValue>0</DefaultValue>"
  "        </Int>"
  "      </ItemDefinitions>"
  "    </AttDef>"
  "    <AttDef Type=\"result\" Abstract=\"True\">"
  "      <ItemDefinitions>"
  "        <Int Name=\"outcome\" Label=\"outcome\" Optional=\"False\" NumberOfRequiredValues=\"1\">"
  "        </Int>"
  "        <String Name=\"log\" Optional=\"True\" NumberOfRequiredValues=\"0\" Extensible=\"True\">"
  "        </String>"
  "      </ItemDefinitions>"
  "    </AttDef>"
  "    <AttDef Type=\"OperationB\" Label=\"A Test Operation\" BaseType=\"operation\">"
  "      <ItemDefinitions>"
  "        <Component Name=\"my component\" Optional=\"False\" NumberOfRequiredValues=\"1\">"
  "          <Accepts>"
  "            <Resource Name=\"ResourceB\" Filter=\"\"/>"
  "          </Accepts>"
  "        </Component>"
  "      </ItemDefinitions>"
  "    </AttDef>"
  "    <AttDef Type=\"result(OperationB)\" BaseType=\"result\">"
  "    </AttDef>"
  "  </Definitions>"
  "</SMTK_AttributeResource>";

const char* OperationB::xmlDescription() const
{
  return operationB_xml;
}
} // namespace

// Test the operation manager's ability to filter available operations based on
// the operation's input resource component type. There are three resources
// (ResourceA, ResourceB and ResourceX) and two operations (OperationA and
// OperationB). ResourceB is derived from Resource A, OperationA accepts
// components from ResourceA and OperationB accepts components from ResourceB.
// We query the operation manager using each of the three resource types and
// test how many operations are available for each type.
int TestAvailableOperations(int /*unused*/, char* /*unused*/[])
{
  // Create a resource manager
  smtk::resource::Manager::Ptr resourceManager = smtk::resource::Manager::create();

  // Register ResourceA
  resourceManager->registerResource<ResourceA>();

  // Register ResourceB
  resourceManager->registerResource<ResourceB>();

  // Register ResourceX
  resourceManager->registerResource<ResourceX>();

  // Create instances of resources A, B and X
  auto resourceA = resourceManager->create<ResourceA>();
  auto resourceB = resourceManager->create<ResourceB>();
  auto resourceX = resourceManager->create<ResourceX>();

  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  // Register OperationA and OperationB
  operationManager->registerOperation<OperationA>("OperationA");
  operationManager->registerOperation<OperationB>("OperationB");

  auto component = MyComponent::create();
  {
    auto tmp = std::static_pointer_cast<smtk::resource::Resource>(resourceA);
    component->setResource(tmp);

    // Query the operation manager for resources that accept our component
    auto availableOperations = operationManager->availableOperations(component);
    smtkTest((availableOperations.size() == 1), "Should be 1 available operation for resourceA.");
  }

  {
    auto tmp = std::static_pointer_cast<smtk::resource::Resource>(resourceB);
    component->setResource(tmp);

    // Query the operation manager for resources that accept our component
    auto availableOperations = operationManager->availableOperations(component);
    smtkTest((availableOperations.size() == 2), "Should be 2 available operations for resourceB.");
  }

  {
    auto tmp = std::static_pointer_cast<smtk::resource::Resource>(resourceX);
    component->setResource(tmp);

    // Query the operation manager for resources that accept our component
    auto availableOperations = operationManager->availableOperations(component);
    smtkTest(availableOperations.empty(), "Should be 0 available operations for resourceX.");
  }

  return 0;
}
