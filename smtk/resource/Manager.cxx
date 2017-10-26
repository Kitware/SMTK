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

#include "smtk/common/UUIDGenerator.h"

namespace smtk
{
namespace resource
{

MetadataContainer Manager::s_metadata;

Manager::Manager()
{
}

Manager::~Manager()
{
}

smtk::resource::ResourcePtr Manager::create(const std::string& uniqueName)
{
  // Locate the metadata associated with this resource type
  auto metadata = s_metadata.get<NameTag>().find(uniqueName);
  if (metadata != s_metadata.get<NameTag>().end())
  {
    // Create the resource using its index
    return this->create(metadata->index());
  }

  return smtk::resource::ResourcePtr();
}

smtk::resource::ResourcePtr Manager::create(const Resource::Index& index)
{
  smtk::common::UUID uuid;

  // Though the chances are super small, we ensure here that the UUID associated
  // to our new resource is unique to the manager's set of resources.
  do
  {
    uuid = smtk::common::UUIDGenerator::instance().random();
  } while (m_resources.find(uuid) != m_resources.end());

  return this->create(index, uuid);
}

smtk::resource::ResourcePtr Manager::create(
  const std::string& uniqueName, const smtk::common::UUID& uuid)
{
  smtk::resource::ResourcePtr resource;

  // Locate the metadata associated with this resource type
  auto metadata = s_metadata.get<NameTag>().find(uniqueName);
  if (metadata != s_metadata.get<NameTag>().end())
  {
    // Create the resource using its index
    resource = metadata->create(uuid);
    this->add(metadata->index(), resource);
  }

  return resource;
}

smtk::resource::ResourcePtr Manager::create(
  const Resource::Index& index, const smtk::common::UUID& uuid)
{
  smtk::resource::ResourcePtr resource;

  // Locate the metadata associated with this resource type
  auto metadata = s_metadata.get<IndexTag>().find(index);
  if (metadata != s_metadata.get<IndexTag>().end())
  {
    // Create the resource with the appropriate UUID
    resource = metadata->create(uuid);
    this->add(index, resource);
  }

  return resource;
}

bool Manager::registerResource(Metadata& metadata)
{
  auto alreadyRegisteredMetadata = s_metadata.get<IndexTag>().find(metadata.index());
  if (alreadyRegisteredMetadata == s_metadata.get<IndexTag>().end())
  {
    auto size = s_metadata.get<IndexTag>().size();
    s_metadata.get<IndexTag>().insert(metadata);
    return s_metadata.get<IndexTag>().size() > size;
  }

  return false;
}

smtk::resource::ResourcePtr Manager::get(const smtk::common::UUID& id)
{
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

std::set<smtk::resource::ResourcePtr> Manager::find(const std::string& uniqueName)
{
  std::set<smtk::resource::ResourcePtr> values;
  std::set<Resource::Index> validIndices;

  auto metadata = s_metadata.get<NameTag>().find(uniqueName);
  if (metadata == s_metadata.get<NameTag>().end())
  {
    return values;
  }

  for (auto& metadatum : s_metadata)
  {
    if (metadatum.m_associatedIndices.find(metadata->index()) !=
      metadatum.m_associatedIndices.end())
    {
      validIndices.insert(metadatum.index());
    }
  }

  typedef Container::index<IndexTag>::type ResourcesByIndex;
  ResourcesByIndex& resources = this->m_resources.get<IndexTag>();
  for (auto& idx : validIndices)
  {
    auto resourceItRange = resources.equal_range(idx);
    values.insert(resourceItRange.first, resourceItRange.second);
  }

  return values;
}

std::set<smtk::resource::ResourcePtr> Manager::find(const Resource::Index& index)
{
  std::set<Resource::Index> validIndices;
  for (auto& metadatum : s_metadata)
  {
    if (metadatum.m_associatedIndices.find(index) != metadatum.m_associatedIndices.end())
    {
      validIndices.insert(metadatum.index());
    }
  }

  std::set<smtk::resource::ResourcePtr> values;

  typedef Container::index<IndexTag>::type ResourcesByIndex;
  ResourcesByIndex& resources = this->m_resources.get<IndexTag>();
  for (auto& idx : validIndices)
  {
    auto resourceItRange = resources.equal_range(idx);
    values.insert(resourceItRange.first, resourceItRange.second);
  }

  return values;
}

smtk::resource::ResourcePtr Manager::read(const Resource::Index& index, const std::string& url)
{
  smtk::resource::ResourcePtr resource;

  // Locate the metadata associated with this resource type
  auto metadata = s_metadata.get<IndexTag>().find(index);
  if (metadata != s_metadata.get<IndexTag>().end())
  {
    // Read in the resource using the provided url
    this->add(index, metadata->read(url));

    if (resource)
    {
      // Assign the resource's location
      resource->setLocation(url);
    }
  }

  return resource;
}

bool Manager::write(const smtk::resource::ResourcePtr& resource, const std::string& url)
{
  // Set the location of the resource to the input url and write
  resource->setLocation(url);
  return this->write(resource);
}

bool Manager::write(const smtk::resource::ResourcePtr& resource)
{
  if (!resource)
  {
    return false;
  }

  // Locate the metadata associated with this resource type
  auto metadata = s_metadata.get<IndexTag>().find(resource->index());
  if (metadata != s_metadata.get<IndexTag>().end())
  {
    // Write out the resource to its url
    return metadata->write(resource);
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
  auto metadata = s_metadata.get<IndexTag>().find(index);
  if (metadata == s_metadata.get<IndexTag>().end())
  {
    return false;
  }

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
  resource->m_manager = this;
  m_resources.insert(resource);
  return true;
}

bool Manager::remove(const smtk::resource::ResourcePtr& resource)
{
  // Find the resource
  typedef Container::index<IdTag>::type ResourcesById;
  ResourcesById& resources = m_resources.get<IdTag>();
  auto resourceIt = resources.find(resource->id());
  if (resourceIt != resources.end())
  {
    // Remove it from the manager's set of resources
    m_resources.erase(resourceIt);

    // Unsert the resource's manager
    (*resourceIt)->m_manager = nullptr;

    return true;
  }

  return false;
}
}
}
