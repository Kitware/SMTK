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
#include "smtk/resource/Manager.h"

#include "smtk/common/testing/cxx/helpers.h"

class ResourceA : public smtk::resource::Resource
{
public:
  smtkTypeMacro(ResourceA);
  smtkCreateMacro(ResourceA);
  smtkSharedFromThisMacro(smtk::resource::Resource);

  smtk::resource::ComponentPtr find(const smtk::common::UUID&) const override
  {
    return smtk::resource::ComponentPtr();
  }

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

  // typedef referring to the parent resource. This is necessary if the derived
  // resource is to be returned by both queries for resources of type <derived>
  // and <base>.
  typedef ResourceA ParentResource;

  smtk::resource::ComponentPtr find(const smtk::common::UUID&) const override
  {
    return smtk::resource::ComponentPtr();
  }

protected:
  ResourceB()
    : ResourceA()
  {
  }
};

int TestResourceManager(int, char** const)
{
  int numResources = 0;
  // Create a resource manager
  smtk::resource::ManagerPtr resourceManager = smtk::resource::Manager::create();

  smtkTest(resourceManager->metadata().empty(), "New resource manager should have no types.");
  smtkTest(resourceManager->resources().empty(), "New resource manager should have no resources.");

  // Create a metadata descriptor for ResourceA and assign it a unique name
  ResourceA::Metadata metadataForResourceA("ResourceA");

  // Assign a create function to the descriptor
  metadataForResourceA.create = [](const smtk::common::UUID&) { return ResourceA::create(); };

  // Register ResourceA
  resourceManager->registerResource<ResourceA>(metadataForResourceA);
  smtkTest(
    resourceManager->metadata().size() == 1, "Resource manager should have registered a type.");

  // Create a new ResourceA type
  auto resourceA1 = resourceManager->create<ResourceA>();
  smtkTest(!!resourceA1, "Failed to create instance A1 of resource A");
  smtkTest(resourceManager->resources().size() == 1, "Resource A1 not added to manager.");

  // Observe resources being added
  int handle;
  handle = resourceManager->observe(
    [&numResources](smtk::resource::Event event, const smtk::resource::Resource::Ptr& rsrc) {
      (void)rsrc;
      numResources += (event == smtk::resource::Event::RESOURCE_ADDED ? +1 : -1);
      std::cout << "Resource count now " << numResources << " rsrc " << rsrc << "\n";
    });
  smtkTest(numResources == 1, "Did not observe new resource being added.");

  // Change its location field (nontrivial due to weak location indexing)
  std::string location = "/path/to/resourceA1";
  resourceA1->setLocation(location);
  smtkTest(resourceA1->location() == location, "Failed to set the location of a resource.");

  // Create another ResourceA type
  auto resourceA2 = resourceManager->create<ResourceA>();
  smtkTest(resourceManager->resources().size() == 2, "Resource A2 not added to manager.");
  smtkTest(numResources == 2, "Did not observe resource A2 being added.");

  // Unregister the observer
  smtkTest(resourceManager->unobserve(handle), "Could not unregister observer.");

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
    // Create a metadata descriptor for ResourceB and it the same unique name as
    // the prior descriptor (bad idea!)
    ResourceB::Metadata metadataForResourceB("ResourceA");

    // Assign a create function to the descriptor
    metadataForResourceB.create = [](const smtk::common::UUID&) { return ResourceB::create(); };

    // Try to register ResourceB
    bool success = resourceManager->registerResource<ResourceB>(metadataForResourceB);
    smtkTest(!success, "Two resource types with the same unique name were registered.");

    // Ensure that the faulty descriptor was not registered
    smtkTest(resourceManager->metadata().size() == 1,
      "Resource manager should still have a single resource type.");
  }

  {
    // Create a metadata descriptor for ResourceB and give it a unique name
    ResourceB::Metadata metadataForResourceB("ResourceB");

    // Assign a create function to the descriptor
    metadataForResourceB.create = [](const smtk::common::UUID&) { return ResourceB::create(); };

    // Try to register ResourceB
    bool success = resourceManager->registerResource<ResourceB>(metadataForResourceB);
    smtkTest(success, "Resource type B should have been registered.");

    // Ensure that the latest descriptor was registered
    smtkTest(resourceManager->metadata().size() == 2,
      "Resource manager should have two resource types registered.");
  }

  // Create a ResourceB instance
  auto resourceB1 = resourceManager->create<ResourceB>();
  smtkTest(resourceManager->resources().size() == 3,
    "Resource manager should be managing three resources.");

  auto resourceBSet = resourceManager->find<ResourceB>();
  smtkTest(resourceBSet.size() == 1,
    "Resource manager should have one resource of type ResourceB registered.");

  auto resourceASet = resourceManager->find<ResourceA>();
  smtkTest(resourceASet.size() == 3,
    "Resource manager should have three resources of type ResourceA registered.");

  return 0;
}
