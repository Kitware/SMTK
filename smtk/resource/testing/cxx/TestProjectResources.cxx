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

namespace
{
class MyResource : public smtk::resource::DerivedFrom<MyResource, smtk::resource::Resource>
{
public:
  smtkTypeMacro(MyResource);
  smtkCreateMacro(MyResource);
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
  MyResource()
    : smtk::resource::DerivedFrom<MyResource, smtk::resource::Resource>()
  {
  }
};

class SubresourceContainer
{
public:
  SubresourceContainer(const std::shared_ptr<smtk::resource::Resource>& resource,
    const std::weak_ptr<smtk::resource::Manager>& weakManager)
    : m_subresource(resource->shared_from_this())
    , m_weakManager(weakManager)
  {
  }

  SubresourceContainer(const std::shared_ptr<smtk::resource::Resource>& resource)
    : SubresourceContainer(resource, std::weak_ptr<smtk::resource::Manager>())
  {
  }

  ~SubresourceContainer()
  {
    if (auto manager = m_weakManager.lock())
    {
      manager->remove(m_subresource);
    }
  }

  const std::shared_ptr<smtk::resource::Resource>& subresource() const { return m_subresource; }

private:
  std::shared_ptr<smtk::resource::Resource> m_subresource;
  std::weak_ptr<smtk::resource::Manager> m_weakManager;
};

class ResourceWithSubresources
  : public smtk::resource::DerivedFrom<ResourceWithSubresources, smtk::resource::Resource>
{
public:
  smtkTypeMacro(ResourceWithSubresources);
  smtkCreateMacro(ResourceWithSubresources);
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

  smtk::resource::ResourcePtr createSubresource()
  {
    smtk::resource::Resource::Ptr resource;
    auto manager = this->manager();
    if (auto manager = this->manager())
    {
      resource = manager->create<MyResource>();
    }
    else
    {
      resource = MyResource::create();
    }

    std::shared_ptr<SubresourceContainer> subresourceContainer(
      new SubresourceContainer(resource, manager));

    std::shared_ptr<smtk::resource::Resource> shared(
      subresourceContainer, subresourceContainer->subresource().get());

    m_resources.push_back(shared);

    return shared;
  }

protected:
  ResourceWithSubresources()
    : smtk::resource::DerivedFrom<ResourceWithSubresources, smtk::resource::Resource>()
  {
  }

private:
  std::vector<smtk::resource::Resource::Ptr> m_resources;
};
}

int TestProjectResources(int /*unused*/, char** const /*unused*/)
{
  // Create a resource manager
  smtk::resource::ManagerPtr resourceManager = smtk::resource::Manager::create();

  smtkTest(resourceManager->metadata().empty(), "New resource manager should have no types.");
  smtkTest(resourceManager->resources().empty(), "New resource manager should have no resources.");

  // Register MyResource
  resourceManager->registerResource<MyResource>();
  smtkTest(
    resourceManager->metadata().size() == 1, "Resource manager should have registered a type.");

  // Register ResourceWithSubresources
  resourceManager->registerResource<ResourceWithSubresources>();
  smtkTest(
    resourceManager->metadata().size() == 2, "Resource manager should have registered a type.");

  smtk::resource::Resource::Ptr subresource;
  {
    // Create an instance of ResourceWithSubresources
    auto resourceWithSubresources = resourceManager->create<ResourceWithSubresources>();
    smtkTest(resourceManager->resources().size() == 1, "Resource not added to manaager");

    // Create a subresource
    subresource = resourceWithSubresources->createSubresource();
    smtkTest(resourceManager->resources().size() == 2, "Subresource not added to manaager");

    // Remove the instance of ResourceWithSubresources
    resourceManager->remove(resourceWithSubresources);

    // Test that the subresource is still managed...
    smtkTest(resourceManager->resources().size() == 1,
      "resourceWithSubresources should be gone, subresource should have remained");
  }

  // ...irrespective of scope
  smtkTest(resourceManager->resources().size() == 1,
    "resourceWithSubresources should be gone, subresource should have remained");

  // Copy the subresource's address for testing purposes
  void* subresource_address = subresource.get();

  // Strip the subresource's shared_ptr of its container
  subresource = subresource->shared_from_this();

  // Test that the underlying subresource is still the same as before...
  smtkTest(
    subresource.get() == subresource_address, "subresource address should have remained the same");
  // ...but it is no longer managed by the resource manager.
  smtkTest(resourceManager->resources().empty(),
    "Destructed resourceWithSubresources should have removed subresource.");

  return 0;
}
