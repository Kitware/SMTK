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
#include "smtk/operation/NewOp.h"
#include "smtk/operation/Observer.h"
#include "smtk/operation/XMLOperator.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Collection.h"
#include "smtk/attribute/IntItem.h"

#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Metadata.h"
#include "smtk/resource/Resource.h"

#include "smtk/common/testing/cxx/helpers.h"

#include "smtk/AutoInit.h"

#include <iostream>

namespace
{
class ResourceA : public smtk::resource::Resource
{
public:
  smtkTypeMacro(ResourceA);
  smtkCreateMacro(ResourceA);
  smtkSharedFromThisMacro(smtk::resource::Resource);
  smtkResourceTypeNameMacro("ResourceA");

  smtk::resource::ComponentPtr find(const smtk::common::UUID&) const override
  {
    return smtk::resource::ComponentPtr();
  }

  std::function<bool(const smtk::resource::ComponentPtr&)> queryOperation(
    const std::string&) const override
  {
    return [](const smtk::resource::ComponentPtr&) { return true; };
  }

  void visit(smtk::resource::Component::Visitor&) const override {}

protected:
  ResourceA()
    : Resource()
  {
  }
};

class ResourceB : public ResourceA
{
public:
  smtkTypeMacro(ResourceB);
  smtkCreateMacro(ResourceA);
  smtkSharedFromThisMacro(smtk::resource::Resource);
  smtkResourceTypeNameMacro("ResourceB");

  // typedef referring to the parent resource. This is necessary if the derived
  // resource is to be returned by both queries for resources of type <derived>
  // and <base>.
  typedef ResourceA ParentResource;

protected:
  ResourceB()
    : ResourceA()
  {
  }
};

class ResourceX : public smtk::resource::Resource
{
public:
  smtkTypeMacro(ResourceX);
  smtkCreateMacro(ResourceX);
  smtkSharedFromThisMacro(smtk::resource::Resource);
  smtkResourceTypeNameMacro("ResourceX");

  smtk::resource::ComponentPtr find(const smtk::common::UUID&) const override
  {
    return smtk::resource::ComponentPtr();
  }

  std::function<bool(const smtk::resource::ComponentPtr&)> queryOperation(
    const std::string&) const override
  {
    return [](const smtk::resource::ComponentPtr&) { return true; };
  }

  void visit(smtk::resource::Component::Visitor&) const override {}

protected:
  ResourceX()
    : Resource()
  {
  }
};

class MyComponent : public smtk::resource::Component
{
public:
  smtkTypeMacro(MyComponent);
  smtkCreateMacro(MyComponent);
  smtkSharedFromThisMacro(smtk::resource::Component);

  smtk::common::UUID id() const override { return myId; }
  void setId(const smtk::common::UUID& anId) override { myId = anId; }

  const smtk::resource::ResourcePtr resource() const override { return myResource; }
  void setResource(smtk::resource::Resource::Ptr& r) { myResource = r; }

private:
  smtk::resource::Resource::Ptr myResource;
  smtk::common::UUID myId;
};

class OperatorA : public smtk::operation::XMLOperator
{
public:
  smtkTypeMacro(OperatorA);
  smtkCreateMacro(OperatorA);
  smtkSharedFromThisMacro(smtk::operation::NewOp);

  OperatorA() {}
  ~OperatorA() override {}

  Result operateInternal() override { return this->createResult(Outcome::SUCCEEDED); }

  const char* xmlDescription() const override;
};

const char operatorA_xml[] =
  "<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
  "<SMTK_AttributeSystem Version=\"3\">"
  "  <Definitions>"
  "    <AttDef Type=\"operator\" Label=\"operator\" Abstract=\"True\">"
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
  "    <AttDef Type=\"OperatorA\" Label=\"A Test Operator\" BaseType=\"operator\">"
  "      <ItemDefinitions>"
  "        <Component Name=\"my component\" Optional=\"False\" NumberOfRequiredValues=\"1\">"
  "          <Accepts>"
  "            <Resource Name=\"ResourceA\" Filter=\"\"/>"
  "          </Accepts>"
  "        </Component>"
  "      </ItemDefinitions>"
  "    </AttDef>"
  "    <AttDef Type=\"result(OperatorA)\" BaseType=\"result\">"
  "    </AttDef>"
  "  </Definitions>"
  "</SMTK_AttributeSystem>";

const char* OperatorA::xmlDescription() const
{
  return operatorA_xml;
}

class OperatorB : public smtk::operation::XMLOperator
{
public:
  smtkTypeMacro(OperatorB);
  smtkCreateMacro(OperatorB);
  smtkSharedFromThisMacro(smtk::operation::NewOp);

  OperatorB() {}
  ~OperatorB() override {}

  Result operateInternal() override { return this->createResult(Outcome::SUCCEEDED); }

  const char* xmlDescription() const override;
};

const char operatorB_xml[] =
  "<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
  "<SMTK_AttributeSystem Version=\"3\">"
  "  <Definitions>"
  "    <AttDef Type=\"operator\" Label=\"operator\" Abstract=\"True\">"
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
  "    <AttDef Type=\"OperatorB\" Label=\"A Test Operator\" BaseType=\"operator\">"
  "      <ItemDefinitions>"
  "        <Component Name=\"my component\" Optional=\"False\" NumberOfRequiredValues=\"1\">"
  "          <Accepts>"
  "            <Resource Name=\"ResourceB\" Filter=\"\"/>"
  "          </Accepts>"
  "        </Component>"
  "      </ItemDefinitions>"
  "    </AttDef>"
  "    <AttDef Type=\"result(OperatorB)\" BaseType=\"result\">"
  "    </AttDef>"
  "  </Definitions>"
  "</SMTK_AttributeSystem>";

const char* OperatorB::xmlDescription() const
{
  return operatorB_xml;
}
}

// Test the operation manager's ability to filter available operators based on
// the operator's input resource component type. There are three resources
// (ResourceA, ResourceB and ResourceX) and two operators (OperatorA and
// OperatorB). ResourceB is derived from Resource A, OperatorA accepts
// components from ResourceA and OperatorB accepts components from ResourceB.
// We query the operation manager using each of the three resource types and
// test how many operators are available for each type.
int TestAvailableOperators(int, char* [])
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

  // Register OperatorA and OperatorB
  operationManager->registerOperator<OperatorA>("OperatorA");
  operationManager->registerOperator<OperatorB>("OperatorB");

  auto component = MyComponent::create();
  {
    auto tmp = std::static_pointer_cast<smtk::resource::Resource>(resourceA);
    component->setResource(tmp);

    // Query the operation manager for resources that accept our component
    auto availableOperators = operationManager->availableOperators(component);
    smtkTest((availableOperators.size() == 1), "Should be 1 available operator for resourceA.");
  }

  {
    auto tmp = std::static_pointer_cast<smtk::resource::Resource>(resourceB);
    component->setResource(tmp);

    // Query the operation manager for resources that accept our component
    auto availableOperators = operationManager->availableOperators(component);
    smtkTest((availableOperators.size() == 2), "Should be 2 available operators for resourceB.");
  }

  {
    auto tmp = std::static_pointer_cast<smtk::resource::Resource>(resourceX);
    component->setResource(tmp);

    // Query the operation manager for resources that accept our component
    auto availableOperators = operationManager->availableOperators(component);
    smtkTest((availableOperators.size() == 0), "Should be 0 available operators for resourceX.");
  }

  return 0;
}
