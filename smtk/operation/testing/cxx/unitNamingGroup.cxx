//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/common/testing/cxx/helpers.h"

#include "smtk/resource/DerivedFrom.h"
#include <smtk/operation/Manager.h>
#include <smtk/operation/XMLOperation.h>
#include <smtk/operation/groups/NamingGroup.h>
#include <smtk/resource/Manager.h>
#include <smtk/resource/Resource.h>

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

const std::string setNameOp_xml = R"***(
  <?xml version="1.0" encoding="utf-8" ?>
  <SMTK_AttributeResource Version="4">
  <Definitions>
  <AttDef Type="operation" Label="operation" Abstract="True">
        <ItemDefinitions>
          <Int Name="debug level" Optional="True">
            <DefaultValue>0</DefaultValue>
          </Int>
        </ItemDefinitions>
      </AttDef>
      <AttDef Type="result" Abstract="True">
        <ItemDefinitions>
          <Int Name="outcome" Label="outcome" Optional="False" NumberOfRequiredValues="1">
          </Int>
          <String Name="log" Optional="True" NumberOfRequiredValues="0" Extensible="True">
          </String>
        </ItemDefinitions>
      </AttDef>
      <AttDef Type="SetNameOp" Label="A Test SetName Operation" BaseType="operation">
      <AssociationsDef Name="model" NumberOfRequiredValues="1" Extensible="False">
        <Accepts>
            <Resource Name="ResourceA" Filter="*"/>
        </Accepts>
      </AssociationsDef>
        <ItemDefinitions>
          <String Name="name" Optional="False" NumberOfRequiredValues="1">
          </String>
        </ItemDefinitions>
      </AttDef>
      <AttDef Type="result(OperationA)" BaseType="result">
      </AttDef>
    </Definitions>
    </SMTK_AttributeResource>
  )***";
class SetNameOp : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(SetNameOp);
  smtkCreateMacro(SetNameOp);
  smtkSharedFromThisMacro(smtk::operation::Operation);

  SetNameOp() = default;
  ~SetNameOp() override = default;

  Result operateInternal() override { return this->createResult(Outcome::SUCCEEDED); }

  const char* xmlDescription() const override { return setNameOp_xml.c_str(); }
};

const std::string nonCompliantOp_xml = R"***(
  <?xml version="1.0" encoding="utf-8" ?>
  <SMTK_AttributeResource Version="4">
    <Definitions>
      <AttDef Type="operation" Label="operation" Abstract="True">
        <ItemDefinitions>
          <Int Name="debug level" Optional="True">
            <DefaultValue>0</DefaultValue>
          </Int>
        </ItemDefinitions>
      </AttDef>
      <AttDef Type="result" Abstract="True">
        <ItemDefinitions>
          <Int Name="outcome" Label="outcome" Optional="False" NumberOfRequiredValues="1">
          </Int>
          <String Name="log" Optional="True" NumberOfRequiredValues="0" Extensible="True">
          </String>
        </ItemDefinitions>
      </AttDef>
      <AttDef Type="NonCompliantOp" Label="A Test SetName Operation" BaseType="operation">
        <ItemDefinitions>
          <Int Name="test" Label="test" Optional="False" NumberOfRequiredValues="1">
          </Int>
        </ItemDefinitions>
      </AttDef>
      <AttDef Type="result(OperationA)" BaseType="result">
      </AttDef>
    </Definitions>
  </SMTK_AttributeResource>
  )***";

class NonCompliantOp : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(NonCompliantOp);
  smtkCreateMacro(NonCompliantOp);
  smtkSharedFromThisMacro(smtk::operation::Operation);

  NonCompliantOp() = default;
  ~NonCompliantOp() override = default;

  Result operateInternal() override { return this->createResult(Outcome::SUCCEEDED); }

  const char* xmlDescription() const override { return nonCompliantOp_xml.c_str(); }
};

int unitNamingGroup(int /*argc*/, char** const /*argv*/)
{
  // Create an operation manager
  smtk::operation::Manager::Ptr operationManager = smtk::operation::Manager::create();

  // Register NonCompliantOp to the manager
  operationManager->registerOperation<NonCompliantOp>("NonCompliantOp");
  operationManager->registerOperation<SetNameOp>("SetNameOp");

  // Create a naming group
  smtk::operation::NamingGroup namingGroup(operationManager);

  // Register OperationA to the group and test for success
  bool success = namingGroup.registerOperation<ResourceA, NonCompliantOp>();
  smtkTest(!success, "Registered a non compliant operation to naming group");

  success = namingGroup.registerOperation<ResourceA, SetNameOp>();
  smtkTest(success, "Compliant operation should have registered")

    // Check that the operation manager has one available group. This shows up only after first successful operation is registered
    auto availableGroups = operationManager->availableGroups();
  smtkTest(availableGroups.size() == 1, "Operation manager should have one available group.");

  // Create a resource manager
  smtk::resource::Manager::Ptr resourceManager = smtk::resource::Manager::create();
  resourceManager->registerResource<ResourceA>();
  auto resourceA = resourceManager->create<ResourceA>();
  auto component = MyComponent::create();
  auto rsrc = std::static_pointer_cast<smtk::resource::Resource>(resourceA);
  component->setResource(rsrc);

  auto index = namingGroup.matchingOperation(*component);
  auto op = operationManager->create(index);
  smtkTest(op != nullptr, "Should return a SetNameOp");
  smtkTest(op->typeName() == "SetNameOp", "Should return a SetNameOp");
  // Query the operation
  return 0;
}
