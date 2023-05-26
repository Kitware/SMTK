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
#include "smtk/operation/Operation.h"
#include "smtk/operation/XMLOperation.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/Resource.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/DerivedFrom.h"
#include "smtk/resource/GarbageCollector.h"
#include "smtk/resource/Manager.h"

#include "smtk/common/TypeName.h"
#include "smtk/common/UUIDGenerator.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <chrono>
#include <memory>
#include <thread>

namespace detail
{
class ResourceA;

class ComponentA : public smtk::resource::Component
{
  friend class ResourceA;

public:
  smtkTypeMacro(ComponentA);
  smtkSuperclassMacro(smtk::resource::PersistentObject);
  smtkSharedFromThisMacro(smtk::resource::PersistentObject);

  const smtk::resource::ResourcePtr resource() const override { return this->m_resource; }

  int value() const { return m_value; }
  void setValue(int v) { m_value = v; }

  const smtk::common::UUID& id() const override { return m_id; }
  bool setId(const smtk::common::UUID& id) override
  {
    m_id = id;
    return true;
  }

private:
  ComponentA(smtk::resource::ResourcePtr resource)
    : m_resource(resource)
  {
  }

  const smtk::resource::ResourcePtr m_resource;

  int m_value;
  smtk::common::UUID m_id;
};

class ResourceA : public smtk::resource::DerivedFrom<ResourceA, smtk::resource::Resource>
{
public:
  smtkTypeMacro(ResourceA);
  smtkCreateMacro(ResourceA);
  smtkSharedFromThisMacro(smtk::resource::PersistentObject);

  ComponentA::Ptr newComponent()
  {
    ComponentA::Ptr shared(new ComponentA(shared_from_this()));
    shared->setId(smtk::common::UUID::random());
    m_components.insert(shared);
    return std::static_pointer_cast<ComponentA>(shared);
  }

  smtk::resource::ComponentPtr find(const smtk::common::UUID& id) const override
  {
    auto it = std::find_if(m_components.begin(), m_components.end(), [&](const ComponentA::Ptr& c) {
      return c->id() == id;
    });
    return (it != m_components.end() ? *it : smtk::resource::ComponentPtr());
  }

  std::function<bool(const smtk::resource::Component&)> queryOperation(
    const std::string& /*unused*/) const override
  {
    return [](const smtk::resource::Component& /*unused*/) { return true; };
  }

  void visit(smtk::resource::Component::Visitor& visitor) const override
  {
    std::for_each(m_components.begin(), m_components.end(), visitor);
  }

  bool erase(const ComponentA::Ptr& comp) { return m_components.erase(comp) > 0; }

  std::size_t size() const { return m_components.size(); }

protected:
  ResourceA()
    : smtk::resource::DerivedFrom<ResourceA, smtk::resource::Resource>()
  {
  }

private:
  std::unordered_set<ComponentA::Ptr> m_components;
};

class DeleterA : public smtk::operation::Operation
{
public:
  smtkTypeMacro(detail::DeleterA);
  smtkSharedPtrCreateMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::Operation);
  static bool s_haveRun;
  ~DeleterA() override = default;

  bool ableToOperate() override { return true; }

protected:
  DeleterA() = default;

  Result operateInternal() override
  {
    bool ok = true;
    auto associations = this->parameters()->associations();
    for (auto it = associations->begin(); it != associations->end(); ++it)
    {
      auto comp = std::dynamic_pointer_cast<ComponentA>(*it);
      if (comp)
      {
        auto rsrc = std::dynamic_pointer_cast<ResourceA>(comp->resource());
        if (rsrc)
        {
          ok &= rsrc->erase(comp);
        }
        else
        {
          ok = false;
        }
      }
      else
      {
        ok = false;
      }
    }
    DeleterA::s_haveRun = true;
    return this->createResult(
      ok ? smtk::operation::Operation::Outcome::SUCCEEDED
         : smtk::operation::Operation::Outcome::FAILED);
  }

  Specification createSpecification() override
  {
    Specification spec = this->createBaseSpecification();

    auto opDef = spec->createDefinition("deleter op", "operation");
    opDef->setBriefDescription("Delete a component.");
    auto opRule = opDef->createLocalAssociationRule();
    opRule->setAcceptsEntries(
      smtk::common::typeName<ResourceA>(), "attribute[type='ComponentA']", true);
    opRule->setIsExtensible(true);

    auto resultDef = spec->createDefinition("result(deleter op)", "result");
    return spec;
  }

  void generateSummary(Result& /*unused*/) override {}
};

bool DeleterA::s_haveRun = false;

void testResourceCount(
  const std::shared_ptr<ResourceA>& resourceA,
  std::size_t expected,
  const char* message)
{
  bool ok = false;
  for (int i = 0; i < 5; ++i)
  {
    if (DeleterA::s_haveRun)
    {
      ok = true;
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  std::cout << "  Resource has " << resourceA->size() << " components.\n";
  if (!ok)
  {
    std::cout << "    (timeout waiting for deleter)\n";
  }
  smtkTest(resourceA->size() == expected, message);
  if (DeleterA::s_haveRun)
  {
    DeleterA::s_haveRun = false;
  }
}

class TestOperation : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(TestOperation);
  smtkCreateMacro(TestOperation);
  smtkSharedFromThisMacro(smtk::operation::Operation);

  TestOperation() = default;
  ~TestOperation() override = default;

  Result operateInternal() override;

  const char* xmlDescription() const override;
};

TestOperation::Result TestOperation::operateInternal()
{
  auto assoc = this->parameters()->associations();
  if (!assoc->isSet(0))
  {
    return this->createResult(Outcome::FAILED);
  }
  auto result = this->createResult(Outcome::SUCCEEDED);
  result->findComponent("modified")->appendValue(assoc->valueAs<smtk::resource::Component>());
  return result;
}

const char testOperationXML[] =
  "<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
  "<SMTK_AttributeSystem Version=\"2\">"
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
  "        <Component Name=\"modified\" NumberOfRequiredValues=\"0\" Extensible=\"1\" "
  "HoldReference=\"1\"/>"
  "      </ItemDefinitions>"
  "    </AttDef>"
  "    <AttDef Type=\"TestOperation\" Label=\"My Operation\" BaseType=\"operation\">"
  "      <AssociationsDef Name=\"components\" Extensible=\"true\" NumberOfRequiredValues=\"0\">"
  "        <Accepts><Resource Name=\"smtk::resource::Resource\" Filter=\"*\"/></Accepts>"
  "      </AssociationsDef>"
  "    </AttDef>"
  "    <AttDef Type=\"result(test op)\" BaseType=\"result\">"
  "    </AttDef>"
  "  </Definitions>"
  "</SMTK_AttributeSystem>";

const char* TestOperation::xmlDescription() const
{
  return testOperationXML;
}
} // namespace detail

using namespace detail;

template<typename T, typename U>
void TestSingle(const T& resourceA, const U& operationManager)
{
  auto resourceManager = resourceA->manager();
  // Do the following in a block so only the resource owns things outside it:
  {
    std::cout << "Test that garbage collection does not accept invalid collectors.\n";
    // Create a new Deleter operation
    auto deleterA = operationManager->template create<DeleterA>();
    bool ok = resourceManager->garbageCollector()->add(deleterA);
    smtkTest(!ok, "Expected adding a deleter with no valid associations to fail.");

    // Create ComponentA instances.
    ComponentA::Ptr componentA0 = resourceA->newComponent();
    ComponentA::Ptr componentA1 = resourceA->newComponent();
    componentA0->setValue(3);
    componentA1->setValue(4);
    deleterA->parameters()->associate(componentA0);

    std::cout << "Test that garbage collection does accept valid collectors.\n";
    ok = resourceManager->garbageCollector()->add(deleterA);
    smtkTest(ok, "Expected adding a deleter with valid associations to succeed.");

    std::cout << "  Resource has " << resourceA->size() << " components.\n";
    std::cout << "  Component has " << componentA0.use_count() << " references.\n";
    smtkTest(resourceA->size() == 2, "Expected 2 components initially.");
    smtkTest(componentA0.use_count() == 2, "Expected 2 shared references initially.");
  }

  std::cout << "Test that garbage collection does happen when it should.\n";
  smtkTest(resourceA->size() == 2, "Expected 2 components after shared-pointer freed.");
  // Now run an operation that will trigger garbage collection.
  auto markerA = operationManager->template create<TestOperation>();
  markerA->parameters()->associate(resourceA);
  markerA->operate();
  testResourceCount(resourceA, 1, "Expected 1 component after mark-modified.");
}

template<typename T, typename U>
void TestMultiple(const T& resourceA, const U& operationManager)
{
  std::cout << "Test that garbage collection does not happen when it shouldn't.\n";
  auto resourceManager = resourceA->manager();
  // Do the following in a block so only the resource owns things outside it:
  {
    ComponentA::Ptr componentA0 = resourceA->newComponent();
    componentA0->setValue(5);
    {
      // Create a new Deleter operation
      auto deleterA = operationManager->template create<DeleterA>();

      // Create another ComponentA instance.
      ComponentA::Ptr componentA1 = resourceA->newComponent();
      componentA1->setValue(6);
      deleterA->parameters()->associate(componentA0);
      deleterA->parameters()->associate(componentA1);

      bool ok = resourceManager->garbageCollector()->add(deleterA);
      smtkTest(ok, "Expected adding a deleter with valid associations to succeed.");

      std::cout << "  Resource has " << resourceA->size() << " components.\n";
      std::cout << "  Component has " << componentA0.use_count() << " references.\n";
      smtkTest(resourceA->size() == 3, "Expected 2 components initially.");
      smtkTest(componentA0.use_count() == 2, "Expected 2 shared references initially.");
    }

    smtkTest(resourceA->size() == 3, "Expected 3 components after shared-pointer freed.");
    // Now run an operation that will trigger garbage collection.
    // This time, nothing should happen since 1 of the shared pointers is still
    // held elsewhere.
    auto markerA = operationManager->template create<TestOperation>();
    markerA->parameters()->associate(resourceA);
    markerA->operate();
    std::cout << "  Resource has " << resourceA->size() << " components.\n";
    testResourceCount(resourceA, 3, "Expected 3 components after first mark-modified.");
  }

  std::cout << "Test that garbage collection works with multiple associations.\n";
  smtkTest(resourceA->size() == 3, "Expected 3 components after shared-pointer freed.");
  // Now run an operation that will trigger garbage collection.
  auto markerA = operationManager->template create<TestOperation>();
  markerA->parameters()->associate(resourceA);
  markerA->operate();
  std::cout << "  Resource has " << resourceA->size() << " components.\n";
  testResourceCount(resourceA, 1, "Expected 1 components after second mark-modified.");
}

int TestGarbageCollector(int /*unused*/, char** const /*unused*/)
{
  // Create managers
  auto resourceManager = smtk::resource::Manager::create();
  auto operationManager = smtk::operation::Manager::create();

  // Register ResourceA
  resourceManager->registerResource<ResourceA>();

  // Register DeleterA
  operationManager->registerOperation<DeleterA>();
  operationManager->registerOperation<TestOperation>();

  // Create a new ResourceA type
  auto resourceA = resourceManager->create<ResourceA>();
  // Manage the created resource.
  resourceManager->add(resourceA);

  TestSingle(resourceA, operationManager);
  TestMultiple(resourceA, operationManager);

  return 0;
}
