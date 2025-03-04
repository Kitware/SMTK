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
#include "smtk/resource/Component.h"
#include "smtk/resource/DerivedFrom.h"
#include "smtk/resource/Manager.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <thread>

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
  smtkCreateMacro(ResourceA);
  smtkSharedFromThisMacro(smtk::resource::PersistentObject);

  // typedef referring to the parent resource. This is necessary if the derived
  // resource is to be returned by both queries for resources of type <derived>
  // and <base>.
  typedef ResourceA ParentResource;

protected:
  ResourceB() = default;
};

class ResourceC : public smtk::resource::DerivedFrom<ResourceC, ResourceB>
{
public:
  smtkTypeMacro(ResourceC);
  smtkCreateMacro(ResourceA);
  smtkSharedFromThisMacro(smtk::resource::PersistentObject);

protected:
  ResourceC() = default;
};
} // namespace

int TestResourceManager(int /*unused*/, char** const /*unused*/)
{
  // Create a resource manager
  smtk::resource::ManagerPtr resourceManager = smtk::resource::Manager::create();

  smtkTest(resourceManager->metadata().empty(), "New resource manager should have no types.");
  smtkTest(resourceManager->empty(), "New resource manager should have no resources.");

  // Register ResourceA
  resourceManager->registerResource<ResourceA>();
  smtkTest(
    resourceManager->metadata().size() == 1, "Resource manager should have registered a type.");

  // Create a new ResourceA type
  auto resourceA1 = resourceManager->create<ResourceA>();
  smtkTest(!!resourceA1, "Failed to create instance A1 of resource A");
  // Add the resource to the manager.
  resourceManager->add(resourceA1);
  smtkTest(resourceManager->size() == 1, "Resource A1 not added to manager.");

  // Observe resources being added
  int numResources = 0;
  auto countingObserver =
    [&numResources](const smtk::resource::Resource& rsrc, smtk::resource::EventType event) {
      (void)rsrc;
      numResources += (event == smtk::resource::EventType::ADDED ? +1 : -1);
      std::cout << "Resource count now " << numResources << " rsrc " << &rsrc << "\n";
    };
  auto handle = resourceManager->observers().insert(countingObserver);
  smtkTest(numResources == 1, "Did not observe new resource being added.");

  // Change its location field (nontrivial due to weak location indexing)
  std::string location = "/path/to/resourceA1";
  resourceA1->setLocation(location);
  smtkTest(resourceA1->location() == location, "Failed to set the location of a resource.");

  // Create another ResourceA type
  auto resourceA2 = resourceManager->create<ResourceA>();
  // Add the resource to the manager.
  resourceManager->add(resourceA2);
  smtkTest(resourceManager->size() == 2, "Resource A2 not added to manager.");
  smtkTest(numResources == 2, "Did not observe resource A2 being added.");

  // Unregister the observer
  smtkTest(resourceManager->observers().erase(handle) == 1, "Could not unregister observer.");

  // Register ResourceC
  resourceManager->registerResource<ResourceC>();
  smtkTest(
    resourceManager->metadata().size() == 2, "Resource manager should have registered a type.");

  // Test that the observer can unregister itself while the observer is being called.
  auto removingObserver = [&handle, &resourceManager](
                            const smtk::resource::Resource& /*unused*/,
                            smtk::resource::EventType /*unused*/) {
    std::cout << "Observer removing self\n";
    resourceManager->observers().erase(handle);
  };

  handle = resourceManager->observers().insert(removingObserver);

  // Create a ResourceC type
  auto resourceC = resourceManager->create<ResourceC>();
  std::cout << "resourceC? " << resourceC << std::endl;
  // Manage resourceC:
  resourceManager->add(resourceC);
  smtkTest(!resourceManager->observers().erase(handle), "Observer did not remove itself.");
  resourceManager->remove(resourceC);

  // Unegister ResourceC
  resourceManager->unregisterResource<ResourceC>();
  smtkTest(
    resourceManager->metadata().size() == 1, "Resource manager should have unregistered a type.");

  // Attempt to set its UUID to that of the first ResourceA type (should fail)
  smtk::common::UUID originalUUID = resourceA2->id();
  smtkTest(!resourceA2->setId(resourceA1->id()), "Resource ID collision allowed.");
  smtkTest(resourceA2->id() == originalUUID, "Resource ID not reset after collision.");

  // Attempt to set its UUID to a heretofore unused UUID (should succeed)
  smtk::common::UUID newUUID;
  do
  {
    newUUID = smtk::common::UUIDGenerator::instance().random();
  } while (newUUID == resourceA1->id() || newUUID == resourceA2->id());

  resourceA2->setId(newUUID);
  smtkTest(resourceA2->id() == newUUID, "Resource ID not properly set.");

  {
    // Try to register ResourceB
    bool success = resourceManager->registerResource<ResourceB>();
    smtkTest(success, "Resource type B should have been registered.");

    // Ensure that the latest descriptor was registered
    smtkTest(
      resourceManager->metadata().size() == 2,
      "Resource manager should have two resource types registered.");
  }

  // Create a ResourceB instance
  auto resourceB1 = resourceManager->create<ResourceB>();
  // Manage resourceB1
  resourceManager->add(resourceB1);
  smtkTest(resourceManager->size() == 3, "Resource manager should be managing three resources.");

  auto resourceBSet = resourceManager->find<ResourceB>();
  smtkTest(
    resourceBSet.size() == 1,
    "Resource manager should have one resource of type ResourceB registered.");

  auto resourceASet = resourceManager->find<ResourceA>();
  smtkTest(
    resourceASet.size() == 3,
    "Resource manager should have three resources of type ResourceA registered.");

  // Test fetching resources by exact index; this will only
  // return instances that are of the given class not including subclasses.
  auto indexA = ResourceA::type_index;
  auto resourcesByIndex = resourceManager->find(indexA, true);
  auto count = resourcesByIndex.size();
  smtkTest(count == 2, "Fetched " << count << " instead of 2 resources by type-index failed.");

  std::vector<std::thread> ts;
  for (int i = 1; i < 100; ++i)
  {
    ts.emplace_back([&resourceManager, i]() {
      if (i % 2 == 0)
      {
        auto rsrc = resourceManager->create<ResourceB>();
        resourceManager->remove(rsrc);
      }
      else
      {
        auto rsrc = resourceManager->create<ResourceA>();
        resourceManager->remove(rsrc);
      }
    });
  }
  for (auto& t : ts)
  {
    t.join();
  }

  return 0;
}
