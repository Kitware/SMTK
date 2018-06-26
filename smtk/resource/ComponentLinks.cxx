//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/resource/ComponentLinks.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/Resource.h"

namespace
{
boost::uuids::uuid linkToResource_ = { { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff } };
smtk::common::UUID linkToResource = smtk::common::UUID(linkToResource_);
}

namespace smtk
{
namespace resource
{
ComponentLinks::ComponentLinks(const Component* component)
  : m_component(component)
{
  // NOTE: When modifying this constructor, do not use the component parameter
  // or m_component field! The parent Component is still in construction and is
  // in an indeterminate state.
}

bool ComponentLinks::isLinkedTo(const ResourcePtr& resource) const
{
  // If the resource pointer cannot be resolved, there is no link.
  if (resource == nullptr)
  {
    return false;
  }

  return this->isLinkedTo(resource->id(), linkToResource);
}

bool ComponentLinks::isLinkedTo(const ComponentPtr& component) const
{
  // If the component pointer cannot be resolved, there is no link.
  if (component == nullptr)
  {
    return false;
  }

  return this->isLinkedTo(component->resource()->id(), component->id());
}

bool ComponentLinks::isLinkedTo(
  const smtk::common::UUID& resourceId, const smtk::common::UUID& componentId) const
{
  // Access the Resource Link data that connects this component's resource to
  // the input resource. If it doesn't exist, then there is no link.
  typedef ResourceLinks::ResourceLinkData ResourceLinkData;
  ResourceLinkData& resourceLinkData = m_component->resource()->links().data();
  auto& resourceLinks = resourceLinkData.get<ResourceLinkData::Right>();

  // All resource links held by a resource have a lhs = the containing resource.
  // We therefore only need to find the resource link with a rhs = the input
  // parameter resource.
  auto resourceRange = resourceLinks.equal_range(resourceId);

  // If the range of resources is empty, then there is no link.
  if (resourceRange.first == resourceRange.second)
  {
    return false;
  }

  // There should be only one link connecting two resources. If there is more
  // than one link, then we are in a bad state.
  assert(std::distance(resourceRange.first, resourceRange.second) == 1);

  // Now that we have the resource link, access the component Link data that
  // connects this component to the input resource. If it doesn't exist, then
  // there is no link.
  const Data& componentLinkData = *resourceRange.first;
  auto linkedTo = componentLinkData.linked_to<Data::Left>(m_component->id());
  return (linkedTo.find(componentId) != linkedTo.end());
}

ComponentLinks::Key ComponentLinks::addLinkTo(
  const ResourcePtr& resource, const ComponentLinks::RoleType& role)
{
  // If the resource pointer cannot be resolved, return a null link.
  if (resource == nullptr)
  {
    return Key();
  }

  return this->addLinkTo(resource, linkToResource, role);
}

ComponentLinks::Key ComponentLinks::addLinkTo(
  const ComponentPtr& component, const ComponentLinks::RoleType& role)
{
  // If the component pointer cannot be resolved, there is no link.
  if (component == nullptr)
  {
    return Key();
  }

  return this->addLinkTo(component->resource(), component->id(), role);
}

ComponentLinks::Key ComponentLinks::addLinkTo(const ResourcePtr& resource,
  const smtk::common::UUID& componentId, const ComponentLinks::RoleType& role)
{
  // Access the Resource Link data that connects this component's resource to
  // the input resource. If it doesn't exist, then create a new resource link.
  typedef ResourceLinks::ResourceLinkData ResourceLinkData;
  ResourceLinkData& resourceLinkData = m_component->resource()->links().data();
  auto& resourceLinks = resourceLinkData.get<ResourceLinkData::Right>();

  // All resource links held by a resource have a lhs = the containing resource.
  // We therefore only need to find the resource link with a rhs = the input
  // parameter resource.
  auto resourceRange = resourceLinks.equal_range(resource->id());

  Data* componentLinkData;

  // We could simply use the ResourceLinkData::insert() method and use whatever
  // iterator is returned (referring to either an accessed or created link), but
  // that would result in our creating a new link base type for every query. We
  // therefore perform the existence check ourselves.

  smtk::common::UUID resourceLinkId;

  // If the range of resources is empty, then there is no link.
  if (resourceRange.first == resourceRange.second)
  {
    resourceLinkId = smtk::common::UUID::random();

    // Keep spinning uuids until one is accepted.
    while (resourceLinkData.has(resourceLinkId))
    {
      resourceLinkId = smtk::common::UUID::random();
    }

    resourceLinkData.insert(ResourceLinkData::LinkBase(resource), resourceLinkId,
      m_component->resource()->id(), resource->id());
    componentLinkData = &resourceLinkData.value(resourceLinkId);
  }
  else
  {
    // There should be only one link connecting two resources. If there is more
    // than one link, then we are in a bad state.
    assert(std::distance(resourceRange.first, resourceRange.second) == 1);

    resourceLinkId = resourceRange.first->id;
    componentLinkData = &resourceLinkData.value(resourceLinkId);
  }

  // Now that we have the resource link, access the component Link data that
  // connects this component to the input resource and create a new link.
  smtk::common::UUID componentLinkId = smtk::common::UUID::random();

  // Keep spinning uuids until one is accepted.
  while (componentLinkData->has(componentLinkId))
  {
    componentLinkId = smtk::common::UUID::random();
  }

  componentLinkData->insert(componentLinkId, m_component->id(), componentId, role);
  return std::make_pair(resourceLinkId, componentLinkId);
}

bool ComponentLinks::removeLink(const ComponentLinks::Key& key)
{
  typedef ResourceLinks::ResourceLinkData ResourceLinkData;
  ResourceLinkData& resourceLinkData = m_component->resource()->links().data();
  if (resourceLinkData.has(key.first) == false)
  {
    return false;
  }
  return resourceLinkData.value(key.first).erase(key.second) > 0;
}

std::pair<ResourcePtr, ComponentLinks::RoleType> ComponentLinks::linkedResource(
  const Key& key) const
{
  typedef ResourceLinks::ResourceLinkData ResourceLinkData;
  const ResourceLinkData& resourceLinkData = m_component->resource()->links().data();
  if (resourceLinkData.has(key.first) == false)
  {
    return std::make_pair(ResourcePtr(), Data::undefinedRole);
  }

  auto& resourceLink = resourceLinkData.value(key.first);
  auto& componentLink = resourceLink.at(key.second);

  // Check that the component link has a resource as its rhs
  if (componentLink.right != linkToResource)
  {
    return std::make_pair(ResourcePtr(), Data::undefinedRole);
  }

  if (resourceLink.resource() == nullptr && m_component->resource()->manager() != nullptr)
  {
    resourceLink.fetch(m_component->resource()->manager());
  }

  return std::make_pair(resourceLink.resource(), componentLink.role);
}

std::pair<ComponentPtr, ComponentLinks::RoleType> ComponentLinks::linkedComponent(
  const Key& key) const
{
  typedef ResourceLinks::ResourceLinkData ResourceLinkData;
  const ResourceLinkData& resourceLinkData = m_component->resource()->links().data();
  if (resourceLinkData.has(key.first) == false)
  {
    return std::make_pair(ComponentPtr(), Data::undefinedRole);
  }

  auto& resourceLink = resourceLinkData.value(key.first);
  auto& componentLink = resourceLink.at(key.second);

  // Check that the component link has a component as its rhs
  if (componentLink.right == linkToResource)
  {
    return std::make_pair(ComponentPtr(), Data::undefinedRole);
  }

  if (resourceLink.resource() == nullptr && m_component->resource()->manager() != nullptr)
  {
    resourceLink.fetch(m_component->resource()->manager());
  }

  if (resourceLink.resource() == nullptr)
  {
    return std::make_pair(ComponentPtr(), componentLink.role);
  }

  return std::make_pair(resourceLink.resource()->find(componentLink.right), componentLink.role);
}

} // namespace resource
} // namespace smtk
