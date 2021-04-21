//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/project/ResourceContainer.h"

#include "smtk/resource/Manager.h"

namespace
{
class ResourceWrapper
{
public:
  ResourceWrapper(
    const std::shared_ptr<smtk::resource::Resource>& resource,
    const std::weak_ptr<smtk::resource::Manager>& manager)
    : m_wrappedResource(resource->shared_from_this())
    , m_manager(manager)
  {
  }

  ResourceWrapper(const std::shared_ptr<smtk::resource::Resource>& resource)
    : ResourceWrapper(resource, std::weak_ptr<smtk::resource::Manager>())
  {
  }

  ~ResourceWrapper()
  {
    if (auto manager = m_manager.lock())
    {
      manager->remove(m_wrappedResource);
    }
  }

  const std::shared_ptr<smtk::resource::Resource>& get() const { return m_wrappedResource; }
  std::shared_ptr<smtk::resource::Resource>& get() { return m_wrappedResource; }

private:
  std::shared_ptr<smtk::resource::Resource> m_wrappedResource;
  std::weak_ptr<smtk::resource::Manager> m_manager;
};
} // namespace

namespace smtk
{
namespace project
{
namespace detail
{
const std::string& role(const smtk::resource::ResourcePtr& r)
{
  return r->properties().get<std::string>()[ResourceContainer::role_name];
}
} // namespace detail

ResourceContainer::~ResourceContainer() {}

bool ResourceContainer::registerResource(const std::string& typeName)
{
  // If we have a manager...
  if (auto manager = m_manager.lock())
  {
    //...we check to see if the type is allowed by the manager.
    auto metadata = manager->metadata().get<smtk::resource::NameTag>().find(typeName);
    if (metadata == manager->metadata().get<smtk::resource::NameTag>().end())
    {
      return false;
    }

    // If the type is present in the manager, add it to our whitelist.
    m_types.insert(typeName);
    return true;
  }

  // Otherwise, return false.
  return false;
}

bool ResourceContainer::registerResource(const smtk::resource::Resource::Index& index)
{
  // If we have a manager...
  if (auto manager = m_manager.lock())
  {
    //...we check to see if the type is allowed by the manager.
    auto metadata = manager->metadata().get<smtk::resource::IndexTag>().find(index);
    if (metadata == manager->metadata().get<smtk::resource::IndexTag>().end())
    {
      return false;
    }

    // If the type is present in the manager, add it to our whitelist.
    m_types.insert(metadata->typeName());
    return true;
  }

  // Otherwise, return false.
  return false;
}

bool ResourceContainer::registerResources(const std::set<std::string>& typeNames)
{
  bool registered = true;
  for (auto& typeName : typeNames)
  {
    registered &= this->registerResource(typeName);
  }
  return registered;
}

bool ResourceContainer::unregisterResource(const std::string& typeName)
{
  // Remove any resources of this type
  typedef Container::index<NameTag>::type ResourcesByName;
  ResourcesByName& resources = m_resources.get<NameTag>();

  auto resourceItRange = resources.equal_range(typeName);
  resources.erase(resourceItRange.first, resourceItRange.second);

  // Remove the typename from the whitelist of types.
  return m_types.erase(typeName) > 0;
}

bool ResourceContainer::unregisterResource(const smtk::resource::Resource::Index& index)
{
  // If we have a manager...
  if (auto manager = m_manager.lock())
  {
    //...we check to see if the type is present in the manager.
    auto metadata = manager->metadata().get<smtk::resource::IndexTag>().find(index);
    if (metadata == manager->metadata().get<smtk::resource::IndexTag>().end())
    {
      return false;
    }

    // If so, use its typename to unregister the resource type.
    return this->unregisterResource(metadata->typeName());
  }

  // Otherwise, return false.
  return false;
}

smtk::resource::ResourcePtr ResourceContainer::get(const smtk::common::UUID& id)
{
  // No type casting is required, so we simply find and return the resource by
  // key.
  typedef Container::index<IdTag>::type ResourcesById;
  ResourcesById& resources = m_resources.get<IdTag>();
  ResourcesById::iterator resourceIt = resources.find(id);
  if (resourceIt != resources.end())
  {
    return *resourceIt;
  }

  return smtk::resource::ResourcePtr();
}

smtk::resource::ConstResourcePtr ResourceContainer::get(const smtk::common::UUID& id) const
{
  // No type casting is required, so we simply find and return the resource by
  // key.
  typedef Container::index<IdTag>::type ResourcesById;
  const ResourcesById& resources = m_resources.get<IdTag>();
  ResourcesById::const_iterator resourceIt = resources.find(id);
  if (resourceIt != resources.end())
  {
    return *resourceIt;
  }

  return smtk::resource::ResourcePtr();
}

smtk::resource::ResourcePtr ResourceContainer::get(const std::string& url)
{
  // No type casting is required, so we simply find and return the resource by
  // key.
  typedef Container::index<LocationTag>::type ResourcesByLocation;
  ResourcesByLocation& resources = m_resources.get<LocationTag>();
  ResourcesByLocation::iterator resourceIt = resources.find(url);
  if (resourceIt != resources.end())
  {
    return *resourceIt;
  }

  return smtk::resource::ResourcePtr();
}

smtk::resource::ConstResourcePtr ResourceContainer::get(const std::string& url) const
{
  // No type casting is required, so we simply find and return the resource by
  // key.
  typedef Container::index<LocationTag>::type ResourcesByLocation;
  const ResourcesByLocation& resources = m_resources.get<LocationTag>();
  ResourcesByLocation::const_iterator resourceIt = resources.find(url);
  if (resourceIt != resources.end())
  {
    return *resourceIt;
  }

  return smtk::resource::ConstResourcePtr();
}

std::set<smtk::resource::ResourcePtr> ResourceContainer::find(const std::string& typeName) const
{
  std::set<smtk::resource::ResourcePtr> values;

  // If we have a manager...
  if (auto manager = m_manager.lock())
  {
    //...we construct a set of valid indices and extract all resources that match
    // these indices.
    std::set<smtk::resource::Resource::Index> validIndices;

    auto metadata = manager->metadata().get<smtk::resource::NameTag>().find(typeName);
    if (metadata == manager->metadata().get<smtk::resource::NameTag>().end())
    {
      return values;
    }

    for (auto& metadatum : manager->metadata())
    {
      if (metadatum.isOfType(metadata->index()))
      {
        validIndices.insert(metadatum.index());
      }
    }

    typedef Container::index<IndexTag>::type ResourcesByIndex;
    const ResourcesByIndex& resources = m_resources.get<IndexTag>();
    for (auto& idx : validIndices)
    {
      auto resourceItRange = resources.equal_range(idx);
      values.insert(resourceItRange.first, resourceItRange.second);
    }
  }
  else
  {
    // Otherwise, we must iterate the list of resources and check if they match
    // the index type (which is less performant).

    for (auto& resource : m_resources)
    {
      if (resource->isOfType(typeName))
      {
        values.insert(resource);
      }
    }
  }

  return values;
}

std::set<smtk::resource::ResourcePtr> ResourceContainer::find(
  const smtk::resource::Resource::Index& index) const
{
  std::set<smtk::resource::ResourcePtr> values;

  // If we have a manager...
  if (auto manager = m_manager.lock())
  {
    //...we construct a set of valid indices and extract all resources that match
    // these indices.
    std::set<smtk::resource::Resource::Index> validIndices;

    for (auto& metadatum : manager->metadata())
    {
      if (metadatum.isOfType(index))
      {
        validIndices.insert(metadatum.index());
      }
    }

    typedef Container::index<IndexTag>::type ResourcesByIndex;
    const ResourcesByIndex& resources = m_resources.get<IndexTag>();
    for (auto& idx : validIndices)
    {
      auto resourceItRange = resources.equal_range(idx);
      values.insert(resourceItRange.first, resourceItRange.second);
    }
  }
  else
  {
    // Otherwise, we must iterate the list of resources and check if they match
    // the index type (which is less performant).

    for (auto& resource : m_resources)
    {
      if (resource->isOfType(index))
      {
        values.insert(resource);
      }
    }
  }

  return values;
}

bool ResourceContainer::add(const smtk::resource::ResourcePtr& resource, const std::string& role)
{
  return this->add(resource->index(), resource, role);
}

bool ResourceContainer::add(
  const smtk::resource::Resource::Index& index,
  const smtk::resource::ResourcePtr& resource,
  const std::string& role)
{
  if (!resource)
  {
    return false;
  }

  // Filter out resources that are not allowed in the container (if a whitelist
  // is provided).
  if (!m_types.empty() && m_types.find(resource->typeName()) == m_types.end())
  {
    return false;
  }

  auto manager = m_manager.lock();
  // If we have a manager...
  if (manager)
  {
    //...and the manager rejects the resource, so do we.
    if (!manager->add(index, resource))
    {
      return false;
    }
  }

  // Assign the resource's role.
  if (detail::role(resource) != role)
  {
    resource->properties().get<std::string>()[role_name] = role;
  }

  // If the project already contains this resource, we are done
  typedef Container::index<IdTag>::type ResourcesById;
  ResourcesById& resources = m_resources.get<IdTag>();
  auto resourceIt = resources.find(resource->id());
  if (resourceIt != resources.end())
  {
    return true;
  }

  // Wrap the resource so it gets removed from management when its shared pointer
  // goes out of scope.
  std::shared_ptr<ResourceWrapper> resourceWrapper(new ResourceWrapper(resource, manager));
  std::shared_ptr<smtk::resource::Resource> shared(resourceWrapper, resourceWrapper->get().get());

  // Insert the resource into the project's set of resources
  m_resources.insert(shared);

  return true;
}

bool ResourceContainer::remove(const smtk::resource::ResourcePtr& resource)
{
  // Find the resource
  typedef Container::index<IdTag>::type ResourcesById;
  ResourcesById& resources = m_resources.get<IdTag>();
  auto resourceIt = resources.find(resource->id());
  if (resourceIt != resources.end())
  {
    smtk::resource::Resource::Ptr rsrc = *resourceIt;

    // Remove it from the project's set of resources
    m_resources.erase(resourceIt);
    return true;
  }

  return false;
}
} // namespace project
} // namespace smtk
