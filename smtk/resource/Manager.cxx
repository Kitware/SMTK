//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/resource/Manager.h"
#include "smtk/resource/GarbageCollector.h"

#include "smtk/io/Logger.h"

#include "smtk/common/UUIDGenerator.h"

namespace smtk
{
namespace resource
{
namespace
{

void InitializeObserver(Manager* mgr, Observer fn)
{
  if (!mgr || !fn)
  {
    return;
  }

  mgr->visit([&fn](const Resource& resource) {
    fn(resource, smtk::resource::EventType::ADDED);
    return common::Processing::CONTINUE;
  });
}
} // namespace

Manager::Manager()
  : m_observers(std::bind(InitializeObserver, this, std::placeholders::_1))
  , m_garbageCollector(GarbageCollector::create())
{
}

Manager::~Manager()
{
  clear();
}

bool Manager::unregisterResource(const std::string& typeName)
{
  // Locate the metadata associated with this resource type
  auto metadata = m_metadata.get<NameTag>().find(typeName);
  if (metadata != m_metadata.get<NameTag>().end())
  {
    m_metadata.get<NameTag>().erase(metadata);
    m_metadataObservers(*metadata, false);
    return true;
  }

  return false;
}

bool Manager::unregisterResource(const Resource::Index& index)
{
  // Locate the metadata associated with this resource type
  auto metadata = m_metadata.get<IndexTag>().find(index);
  if (metadata != m_metadata.get<IndexTag>().end())
  {
    m_metadata.get<IndexTag>().erase(metadata);
    m_metadataObservers(*metadata, false);
    return true;
  }

  return false;
}

bool Manager::registered(const std::string& typeName) const
{
  const auto metadata = m_metadata.get<NameTag>().find(typeName);
  return metadata != m_metadata.get<NameTag>().end();
}

bool Manager::registered(const Resource::Index& index) const
{
  const auto metadata = m_metadata.get<IndexTag>().find(index);
  return metadata != m_metadata.get<IndexTag>().end();
}

void Manager::clear()
{
  ScopedLockGuard guard(m_lock, LockType::Write);
  for (auto resourceIt = m_resources.begin(); resourceIt != m_resources.end();)
  {
    Resource::Ptr resource = *resourceIt;
    resourceIt = m_resources.erase(resourceIt);

    m_observers(*resource, smtk::resource::EventType::REMOVED);
  }
}

smtk::resource::ResourcePtr Manager::create(
  const std::string& typeName,
  const std::shared_ptr<smtk::common::Managers>& managers)
{
  // Locate the metadata associated with this resource type
  auto metadata = m_metadata.get<NameTag>().find(typeName);
  if (metadata != m_metadata.get<NameTag>().end())
  {
    // Create the resource using its index
    return this->create(metadata->index(), managers);
  }

  return smtk::resource::ResourcePtr();
}

smtk::resource::ResourcePtr Manager::create(
  const Resource::Index& index,
  const std::shared_ptr<smtk::common::Managers>& managers)
{
  smtk::common::UUID uuid;

  // Though the chances are super small, we ensure here that the UUID associated
  // to our new resource is unique to the manager's set of resources.
  {
    ScopedLockGuard guard(m_lock, LockType::Read);
    do
    {
      uuid = smtk::common::UUIDGenerator::instance().random();
    } while (m_resources.find(uuid) != m_resources.end());
  }

  return this->create(index, uuid, managers);
}

smtk::resource::ResourcePtr Manager::create(
  const std::string& typeName,
  const smtk::common::UUID& uuid,
  const std::shared_ptr<smtk::common::Managers>& managers)
{
  smtk::resource::ResourcePtr resource;

  // Locate the metadata associated with this resource type
  auto metadata = m_metadata.get<NameTag>().find(typeName);
  if (metadata != m_metadata.get<NameTag>().end() && !!metadata->create)
  {
    // Create the resource using its index
    resource = metadata->create(uuid, managers);
    this->add(metadata->index(), resource);
  }

  return resource;
}

smtk::resource::ResourcePtr Manager::create(
  const Resource::Index& index,
  const smtk::common::UUID& uuid,
  const std::shared_ptr<smtk::common::Managers>& managers)
{
  smtk::resource::ResourcePtr resource;

  // Locate the metadata associated with this resource type
  auto metadata = m_metadata.get<IndexTag>().find(index);
  if (metadata != m_metadata.get<IndexTag>().end() && !!metadata->create)
  {
    // Create the resource with the appropriate UUID
    resource = metadata->create(uuid, managers);
    this->add(index, resource);
  }

  return resource;
}

bool Manager::registerResource(Metadata&& metadata)
{
  auto alreadyRegisteredMetadata = m_metadata.get<IndexTag>().find(metadata.index());
  if (alreadyRegisteredMetadata == m_metadata.get<IndexTag>().end())
  {
    auto inserted = m_metadata.get<IndexTag>().insert(metadata);
    if (inserted.second)
    {
      m_metadataObservers(*inserted.first, true);
      return true;
    }
  }

  return false;
}

smtk::resource::ResourcePtr Manager::get(const smtk::common::UUID& id)
{
  ScopedLockGuard guard(m_lock, LockType::Read);
  // No type casting is required, so we simply find and return the resource by
  // key.
  typedef Container::index<IdTag>::type ResourcesById;
  ResourcesById& resources = m_resources.get<IdTag>();
  ResourcesById::iterator resourceIt = resources.find(id);
  if (resourceIt != resources.end())
  {
    return (*resourceIt)->shared_from_this();
  }

  return smtk::resource::ResourcePtr();
}

smtk::resource::ConstResourcePtr Manager::get(const smtk::common::UUID& id) const
{
  ScopedLockGuard guard(m_lock, LockType::Read);
  // No type casting is required, so we simply find and return the resource by
  // key.
  typedef Container::index<IdTag>::type ResourcesById;
  const ResourcesById& resources = m_resources.get<IdTag>();
  ResourcesById::const_iterator resourceIt = resources.find(id);
  if (resourceIt != resources.end())
  {
    return (*resourceIt)->shared_from_this();
  }

  return smtk::resource::ConstResourcePtr();
}

smtk::resource::ResourcePtr Manager::get(const std::string& url)
{
  ScopedLockGuard guard(m_lock, LockType::Read);
  // No type casting is required, so we simply find and return the resource by
  // key.
  typedef Container::index<LocationTag>::type ResourcesByLocation;
  ResourcesByLocation& resources = m_resources.get<LocationTag>();
  ResourcesByLocation::iterator resourceIt = resources.find(url);
  if (resourceIt != resources.end())
  {
    return (*resourceIt)->shared_from_this();
  }

  return smtk::resource::ResourcePtr();
}

smtk::resource::ConstResourcePtr Manager::get(const std::string& url) const
{
  ScopedLockGuard guard(m_lock, LockType::Read);
  // No type casting is required, so we simply find and return the resource by
  // key.
  typedef Container::index<LocationTag>::type ResourcesByLocation;
  const ResourcesByLocation& resources = m_resources.get<LocationTag>();
  ResourcesByLocation::const_iterator resourceIt = resources.find(url);
  if (resourceIt != resources.end())
  {
    return (*resourceIt)->shared_from_this();
  }

  return smtk::resource::ConstResourcePtr();
}

std::set<smtk::resource::ResourcePtr> Manager::find(const std::string& typeName)
{
  ScopedLockGuard guard(m_lock, LockType::Read);
  std::set<smtk::resource::ResourcePtr> values;

  // If the typename matches the abstract smtk::resource::Resource, a search
  // through the metadata to find the corresponding type index will fail because
  // there is no metadata for this class. That is ok, though, because if we are
  // looking for all resources, there's no need to perform a lookup.
  if (typeName == smtk::common::typeName<smtk::resource::Resource>())
  {
    for (const auto& resource : m_resources)
    {
      values.insert(resource);
    }
    return values;
  }

  std::set<Resource::Index> validIndices;

  auto metadata = m_metadata.get<NameTag>().find(typeName);
  if (metadata == m_metadata.get<NameTag>().end())
  {
    return values;
  }

  for (const auto& metadatum : m_metadata)
  {
    if (metadatum.isOfType(metadata->index()))
    {
      validIndices.insert(metadatum.index());
    }
  }

  typedef Container::index<IndexTag>::type ResourcesByIndex;
  ResourcesByIndex& resources = m_resources.get<IndexTag>();
  for (const auto& idx : validIndices)
  {
    auto resourceItRange = resources.equal_range(idx);
    values.insert(resourceItRange.first, resourceItRange.second);
  }

  return values;
}

std::set<smtk::resource::ResourcePtr> Manager::find(const Resource::Index& index, bool strict)
{
  std::set<Resource::Index> validIndices;
  if (strict)
  {
    validIndices.insert(index);
  }
  else
  {
    for (const auto& metadatum : m_metadata)
    {
      if (metadatum.isOfType(index))
      {
        validIndices.insert(metadatum.index());
      }
    }
  }

  std::set<smtk::resource::ResourcePtr> values;

  {
    ScopedLockGuard guard(m_lock, LockType::Read);
    typedef Container::index<IndexTag>::type ResourcesByIndex;
    ResourcesByIndex& resources = m_resources.get<IndexTag>();
    for (const auto& idx : validIndices)
    {
      auto resourceItRange = resources.equal_range(idx);
      values.insert(resourceItRange.first, resourceItRange.second);
    }
  }

  return values;
}

smtk::resource::ResourcePtr Manager::read(
  const std::string& typeName,
  const std::string& url,
  const std::shared_ptr<smtk::common::Managers>& managers)
{
  smtk::resource::ResourcePtr resource;

  // Locate the metadata associated with this resource type
  auto metadata = m_metadata.get<NameTag>().find(typeName);
  if (metadata != m_metadata.get<NameTag>().end() && !!metadata->read)
  {
    // Read in the resource using the provided url
    resource = metadata->read(url, managers);
  }
  else
  {
    // If a resource type is not identified using this type name, we check the
    // map of legacy readers to see if this name is a legacy name for a resource type.
    auto search = m_legacyReaders.find(typeName);
    if (search != m_legacyReaders.end())
    {
      // Read in the resource using the provided url
      resource = search->second(url);
    }
  }

  if (resource)
  {
    // Add the resource to be tracked by this manager
    this->add(resource);

    // Assign the resource's location
    resource->setLocation(url);
  }

  return resource;
}

smtk::resource::ResourcePtr Manager::read(
  const Resource::Index& index,
  const std::string& url,
  const std::shared_ptr<smtk::common::Managers>& managers)
{
  smtk::resource::ResourcePtr resource;

  // Locate the metadata associated with this resource type
  auto metadata = m_metadata.get<IndexTag>().find(index);
  if (metadata != m_metadata.get<IndexTag>().end())
  {
    // Read in the resource using the provided url
    resource = metadata->read(url, managers);
  }

  if (resource)
  {
    // Add the resource to be tracked by this manager
    this->add(index, resource);

    // Assign the resource's location
    resource->setLocation(url);

    // Set the resource as unmodified from its persistent (i.e. on-disk) state
    resource->setClean(true);
  }

  return resource;
}

bool Manager::write(
  const smtk::resource::ResourcePtr& resource,
  const std::string& url,
  const std::shared_ptr<smtk::common::Managers>& managers)
{
  // Set the location of the resource to the input url and write
  resource->setLocation(url);
  return this->write(resource, managers);
}

bool Manager::write(
  const smtk::resource::ResourcePtr& resource,
  const std::shared_ptr<smtk::common::Managers>& managers)
{
  if (!resource)
  {
    return false;
  }

  // Locate the metadata associated with this resource type
  auto metadata = m_metadata.get<IndexTag>().find(resource->index());
  if (metadata != m_metadata.get<IndexTag>().end() && !!metadata->write)
  {
    // Write out the resource to its url
    bool success = metadata->write(resource, managers);

    // If the write was successful, mark the resource as unmodified from its
    // persistent (i.e. on-disk) state
    if (success)
    {
      resource->setClean(true);
    }
    return success;
  }

  return false;
}

bool Manager::add(const smtk::resource::ResourcePtr& resource)
{
  // If the resource is null, do not add
  if (!resource)
  {
    return false;
  }

  return this->add(resource->index(), resource);
}

bool Manager::add(const Resource::Index& index, const smtk::resource::ResourcePtr& resource)
{
  // If the resource is null, do not add
  if (!resource)
  {
    return false;
  }

  // If the manager cannot manage a resource of this type, do not add
  auto metadata = m_metadata.get<IndexTag>().find(index);
  if (metadata == m_metadata.get<IndexTag>().end())
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Resource manager " << this << " is refusing to add resource " << resource << " of type \""
                          << resource->typeName() << "\" "
                          << "as that type has not been registered.");
    return false;
  }

  {
    ScopedLockGuard guard(m_lock, LockType::Write);
    // If the manager already contains this resource, we are done
    typedef Container::index<IdTag>::type ResourcesById;
    ResourcesById& resources = m_resources.get<IdTag>();
    auto resourceIt = resources.find(resource->id());
    if (resourceIt != resources.end())
    {
      return true;
    }

    // Assign the resource's manager and insert it into the manager's set of
    // resources
    resource->m_manager = this->shared_from_this();
    m_resources.insert(resource);

    // Resolve resource surrogate links between the new resource and currently
    // managed resources.
    for (const auto& rsrc : m_resources)
    {
      resource->links().resolve(rsrc);
      rsrc->links().resolve(resource);
    }
  }

  // Tell observers we just added a resource:
  m_observers(*resource, smtk::resource::EventType::ADDED);

  return true;
}

bool Manager::remove(const smtk::resource::ResourcePtr& resource)
{
  bool didRemove = false;
  {
    ScopedLockGuard guard(m_lock, LockType::Write);
    // Find the resource
    typedef Container::index<IdTag>::type ResourcesById;
    ResourcesById& resources = m_resources.get<IdTag>();
    auto resourceIt = resources.find(resource->id());
    if (resourceIt != resources.end())
    {
      Resource::Ptr rsrc = *resourceIt;

      // Remove it from the manager's set of resources
      m_resources.erase(resourceIt);

      // Clear the resource's manager
      rsrc->m_manager = Ptr();

      didRemove = true;
    }
  }

  if (didRemove)
  {
    // Tell observers we have removed it *after* unlocking the manager.
    m_observers(*resource, smtk::resource::EventType::REMOVED);
  }

  return didRemove;
}

bool Manager::addLegacyReader(
  const std::string& alias,
  const std::function<ResourcePtr(const std::string&)>& read)
{
  if (m_legacyReaders.find(alias) != m_legacyReaders.end())
  {
    // This alias is already registered to a resource type.
    return false;
  }

  m_legacyReaders[alias] = read;
  return true;
}

void Manager::reviseId(const Resource::SetId& source, const Resource::SetId& destination)
{
  ScopedLockGuard guard(m_lock, LockType::Write);

  typedef Container::index<IdTag>::type ResourcesById;
  ResourcesById& resources = m_resources.get<IdTag>();
  ResourcesById::iterator resourceIt = resources.find(source.id());

  // try to modify the id, restore it in case of collisions
  resources.modify(resourceIt, destination, source);
}

void Manager::reviseLocation(
  const smtk::common::UUID& uid,
  const Resource::SetLocation& source,
  const Resource::SetLocation& destination)
{
  ScopedLockGuard guard(m_lock, LockType::Write);

  typedef Container::index<LocationTag>::type ResourcesByLocation;
  ResourcesByLocation& resources = m_resources.get<LocationTag>();
  // Multiple resources can have the same location (especially an empty "" location).
  // Find the resource with the given ID and modify it:
  auto resourceItRange = resources.equal_range(source.location());
  for (auto entry = resourceItRange.first; entry != resourceItRange.second; ++entry)
  {
    if (entry->get()->id() == uid)
    {
      resources.modify(entry, destination, source);
      break;
    }
  }
}

smtk::common::Termination Manager::visit(const ResourceVisitor& visitor) const
{
  ScopedLockGuard guard(m_lock, LockType::Read);
  const auto& resourcesById = m_resources.get<IdTag>();
  for (const auto& resource : resourcesById)
  {
    if (visitor(*resource) == smtk::common::Processing::STOP)
    {
      return smtk::common::Termination::EARLY;
    }
  }
  return smtk::common::Termination::NORMAL;
}
} // namespace resource
} // namespace smtk
