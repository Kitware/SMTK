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

class ResourceA : public smtk::resource::Resource
{
public:
  smtkTypeMacro(ResourceA);
  smtkCreateMacro(ResourceA);
  smtkSharedFromThisMacro(smtk::resource::Resource);

  Type type() const override { return Type::ATTRIBUTE; }

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
  smtkCreateMacro(ResourceB);
  smtkSharedFromThisMacro(smtk::resource::Resource);

  // typedef referring to the parent resource. This is necessary if the derived
  // resource is to be returned by both queries for resources of type <derived>
  // and <base>.
  typedef ResourceA ParentResource;

  Type type() const override { return Type::ATTRIBUTE; }

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
  // Create a resource manager
  smtk::resource::ManagerPtr resourceManager = smtk::resource::Manager::create();

  if (resourceManager->metadata().empty() == false)
  {
    std::cout << "new resource manager should have no resources registered" << std::endl;
    return 1;
  }

  if (resourceManager->resources().empty() == false)
  {
    std::cout << "new resource manager should be managing no resources" << std::endl;
    return 1;
  }

  // Create a metadata descriptor for ResourceA and assign it a unique name

  ResourceA::Metadata metadataForResourceA("ResourceA");

  // Assign a create function to the descriptor
  metadataForResourceA.create = [](const smtk::common::UUID&) { return ResourceA::create(); };

  // Register ResourceA
  resourceManager->registerResource<ResourceA>(metadataForResourceA);
  if (resourceManager->metadata().size() != 1)
  {
    std::cout << "resource manager should have one resource registered" << std::endl;
    return 1;
  }

  // Create a new ResourceA type
  auto resourceA1 = resourceManager->create<ResourceA>();
  if (!resourceA1)
  {
    std::cout << "failed to create an instance of resource A" << std::endl;
    return 1;
  }
  if (resourceManager->resources().size() != 1)
  {
    std::cout << "resource manager should be managing one resource" << std::endl;
    return 1;
  }

  // Change its location field (nontrivial due to weak location indexing)
  std::string location = "/path/to/resourceA1";
  resourceA1->setLocation(location);
  if (resourceA1->location() != location)
  {
    std::cout << "failed to set the location of a resource" << std::endl;
    return 1;
  }

  // Create another ResourceA type
  auto resourceA2 = resourceManager->create<ResourceA>();
  if (resourceManager->resources().size() != 2)
  {
    std::cout << "resource manager should be managing two resources" << std::endl;
    return 1;
  }

  // Attempt to set its UUID to that of the first ResourceA type (should fail)
  smtk::common::UUID originalUUID = resourceA2->id();
  if (resourceA2->setId(resourceA1->id()) == true)
  {
    std::cout << "resource id collision" << std::endl;
    return 1;
  }

  if (resourceA2->id() != originalUUID)
  {
    std::cout << "resource id not properly reset after a collision" << std::endl;
    return 1;
  }

  // Attempt to set its UUID to a heretofore unused UUID (should succeed)
  smtk::common::UUID newUUID;
  do
  {
    newUUID = smtk::common::UUIDGenerator::instance().random();
  } while (newUUID == resourceA1->id() || newUUID == resourceA2->id());

  resourceA2->setId(newUUID);
  if (resourceA2->id() != newUUID)
  {
    std::cout << "resource id not properly set" << std::endl;
    return 1;
  }

  {
    // Create a metadata descriptor for ResourceB and it the same unique name as
    // the prior descriptor (bad idea!)
    ResourceB::Metadata metadataForResourceB("ResourceA");

    // Assign a create function to the descriptor
    metadataForResourceB.create = [](const smtk::common::UUID&) { return ResourceB::create(); };

    // Try to register ResourceB
    bool success = resourceManager->registerResource<ResourceB>(metadataForResourceB);
    if (success == true)
    {
      std::cout << "Two resources with the same unique name have been registered" << std::endl;
      return 1;
    }

    // Ensure that the faulty descriptor was not registered
    if (resourceManager->metadata().size() != 1)
    {
      std::cout << "resource manager should still have one resource registered" << std::endl;
      return 1;
    }
  }

  {
    // Create a metadata descriptor for ResourceB and give it a unique name
    ResourceB::Metadata metadataForResourceB("ResourceB");

    // Assign a create function to the descriptor
    metadataForResourceB.create = [](const smtk::common::UUID&) { return ResourceB::create(); };

    // Try to register ResourceB
    bool success = resourceManager->registerResource<ResourceB>(metadataForResourceB);
    if (success == false)
    {
      std::cout << "Resource should be registered" << std::endl;
      return 1;
    }

    // Ensure that the latest descriptor was registered
    if (resourceManager->metadata().size() != 2)
    {
      std::cout << "resource manager should have two resources registered" << std::endl;
      return 1;
    }
  }

  // Create a ResourceB type
  auto resourceB1 = resourceManager->create<ResourceB>();
  if (resourceManager->resources().size() != 3)
  {
    std::cout << "resource manager should be managing three resources" << std::endl;
    return 1;
  }

  auto resourceBSet = resourceManager->find<ResourceB>();

  if (resourceBSet.size() != 1)
  {
    std::cout << "resource manager should have one resource of type ResourceB registered"
              << std::endl;
    return 1;
  }

  auto resourceASet = resourceManager->find<ResourceA>();

  if (resourceASet.size() != 3)
  {
    std::cout << "resource manager should have three resources of type ResourceA registered"
              << std::endl;
    return 1;
  }

  return 0;
}
