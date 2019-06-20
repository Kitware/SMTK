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
#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

namespace
{
smtk::common::UUID linkToResource = smtk::common::UUID::null();
}

namespace smtk
{
namespace resource
{
bool Links::isLinkedTo(const ResourcePtr& rhs1, const RoleType& role) const
{
  return this->isLinkedTo(this->leftHandSideResource(), this->leftHandSideComponentId(), rhs1->id(),
    linkToResource, role);
}

bool Links::isLinkedTo(const ComponentPtr& rhs2, const RoleType& role) const
{
  return this->isLinkedTo(this->leftHandSideResource(), this->leftHandSideComponentId(),
    rhs2->resource()->id(), rhs2->id(), role);
}

Links::Key Links::addLinkTo(const ResourcePtr& rhs1, const RoleType& role)
{
  return this->addLinkTo(
    this->leftHandSideResource(), this->leftHandSideComponentId(), rhs1, linkToResource, role);
}

Links::Key Links::addLinkTo(const ComponentPtr& rhs2, const RoleType& role)
{
  return this->addLinkTo(this->leftHandSideResource(), this->leftHandSideComponentId(),
    rhs2->resource(), rhs2->id(), role);
}

PersistentObjectSet Links::linkedFrom(const RoleType& role) const
{
  return this->linkedFrom(this->leftHandSideResource(), this->leftHandSideComponentId(), role);
}

PersistentObjectSet Links::linkedFrom(const ResourcePtr& rhs1, const RoleType& role) const
{
  return this->linkedFrom(
    rhs1, this->leftHandSideResource(), this->leftHandSideComponentId(), role);
}

bool Links::removeLink(const Key& key)
{
  return this->removeLink(this->leftHandSideResource(), key);
}

bool Links::removeLinksTo(const ResourcePtr& rhs1, const RoleType& role)
{
  return this->removeLinksTo(this->leftHandSideResource(), this->leftHandSideComponentId(),
    rhs1->id(), linkToResource, role);
}

bool Links::removeLinksTo(const ComponentPtr& rhs2, const RoleType& role)
{
  return this->removeLinksTo(this->leftHandSideResource(), this->leftHandSideComponentId(),
    rhs2->resource()->id(), rhs2->id(), role);
}

std::pair<PersistentObjectPtr, Links::RoleType> Links::linkedObjectAndRole(const Key& key) const
{
  return this->linkedObjectAndRole(this->leftHandSideResource(), key);
}

PersistentObjectSet Links::linkedTo(const RoleType& role) const
{
  return this->linkedTo(this->leftHandSideResource(), this->leftHandSideComponentId(), role);
}

const smtk::common::UUID& Links::leftHandSideComponentId() const
{
  return linkToResource;
}

bool Links::isLinkedTo(const Resource* lhs1, const smtk::common::UUID& lhs2,
  const smtk::common::UUID& rhs1, const smtk::common::UUID& rhs2, const RoleType& role) const
{
  // Access the Resource Link data that connects this component's resource to
  // the input resource. If it doesn't exist, then there is no link.
  typedef Resource::Links::ResourceLinkData ResourceLinkData;
  const ResourceLinkData& resourceLinkData = lhs1->links().data();
  auto& resourceLinks = resourceLinkData.get<ResourceLinkData::Right>();

  // All resource links held by a resource have a lhs = the containing resource.
  // We therefore only need to find the resource link with a rhs = the input
  // parameter resource.
  auto resourceRange = resourceLinks.equal_range(rhs1);

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
  const Component::Links::Data& componentLinkData = *resourceRange.first;
  auto linkedTo = componentLinkData.linked_to<Component::Links::Data::Left>(lhs2, role);
  return (linkedTo.find(rhs2) != linkedTo.end());
}

Links::Key Links::addLinkTo(Resource* lhs1, const smtk::common::UUID& lhs2, const ResourcePtr& rhs1,
  const smtk::common::UUID& rhs2, const RoleType& role)
{
  // Access the Resource Link data that connects this component's resource to
  // the input resource. If it doesn't exist, then create a new resource link.
  typedef Resource::Links::ResourceLinkData ResourceLinkData;
  ResourceLinkData& resourceLinkData = lhs1->links().data();
  auto& resourceLinks = resourceLinkData.get<ResourceLinkData::Right>();

  // All resource links held by a resource have a lhs = the containing resource.
  // We therefore only need to find the resource link with a rhs = the input
  // parameter resource.
  auto resourceRange = resourceLinks.equal_range(rhs1->id());

  Component::Links::Data* componentLinkData;

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

    resourceLinkData.insert(
      ResourceLinkData::LinkBase(rhs1), resourceLinkId, lhs1->id(), rhs1->id());
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

  componentLinkData->insert(componentLinkId, lhs2, rhs2, role);
  return std::make_pair(resourceLinkId, componentLinkId);
}

PersistentObjectSet Links::linkedTo(
  const Resource* lhs1, const smtk::common::UUID& lhs2, const RoleType& role) const
{
  // Access the Resource Link data that connects this component's resource to
  // the input resource. If it doesn't exist, then there is no link.
  typedef Resource::Links::ResourceLinkData ResourceLinkData;
  const ResourceLinkData& resourceLinkData = lhs1->links().data();

  PersistentObjectSet objectSet;
  for (auto& resourceLink : resourceLinkData)
  {
    // Access the resource associated with this link. If it cannot be resolved,
    // there's not much we can do with it.
    auto rhs1 = resourceLink.resource();

    if (rhs1 == nullptr)
    {
      continue;
    }

    auto& data = resourceLink.get<Component::Links::Data::Left>();
    auto range = data.equal_range(std::make_tuple(lhs2, role));

    for (auto& link = range.first; link != range.second; ++link)
    {
      if (link->right == linkToResource)
      {
        objectSet.insert(rhs1);
      }
      else
      {
        objectSet.insert(rhs1->find(link->right));
      }
    }
  }

  return objectSet;
}

PersistentObjectSet Links::linkedFrom(const Resource::Ptr& lhs1, const Resource* rhs1,
  const smtk::common::UUID& rhs2, const RoleType& role) const
{
  PersistentObjectSet objectSet;

  // Access the Resource Link data that connects this component's resource to
  // the input resource. If it doesn't exist, then there is no link.
  typedef Resource::Links::ResourceLinkData ResourceLinkData;
  const auto& resourceLinks = lhs1->links().data().get<ResourceLinkData::Right>();

  // All resource links held by a resource have a lhs = the containing resource.
  // We therefore only need to find the resource link with a rhs = the input
  // parameter resource.
  auto resourceRange = resourceLinks.equal_range(rhs1->id());

  // If the range of resources is empty, then there is no link.
  if (resourceRange.first == resourceRange.second)
  {
    return objectSet;
  }

  // There should be only one link connecting two resources. If there is more
  // than one link, then we are in a bad state.
  assert(std::distance(resourceRange.first, resourceRange.second) == 1);

  // Now that we have the resource link, access the component Link data that
  // connects this component to the input resource. If it doesn't exist, then
  // there is no link.
  const Component::Links::Data& componentLinkData = *resourceRange.first;

  auto& data = componentLinkData.get<Component::Links::Data::Right>();
  auto range = data.equal_range(std::make_tuple(rhs2, role));

  for (auto& link = range.first; link != range.second; ++link)
  {
    if (link->left == linkToResource)
    {
      objectSet.insert(lhs1);
    }
    else
    {
      objectSet.insert(lhs1->find(link->left));
    }
  }

  return objectSet;
}

PersistentObjectSet Links::linkedFrom(
  const Resource* rhs1, const smtk::common::UUID& rhs2, const RoleType& role) const
{
  PersistentObjectSet objectSet;

  // Access the manager managing this resource. If none exists, there's not much
  // we can do.
  smtk::resource::Manager::Ptr manager = rhs1->manager();
  if (manager == nullptr)
  {
    return objectSet;
  }

  for (auto& lhs1 : manager->resources())
  {
    PersistentObjectSet objectSetForResource = this->linkedFrom(lhs1, rhs1, rhs2, role);
    objectSet.insert(objectSetForResource.begin(), objectSetForResource.end());
  }
  return objectSet;
}

bool Links::removeLink(Resource* lhs1, const Links::Key& key)
{
  typedef Resource::Links::ResourceLinkData ResourceLinkData;
  ResourceLinkData& resourceLinkData = lhs1->links().data();
  if (resourceLinkData.has(key.first) == false)
  {
    return false;
  }
  bool returnValue = resourceLinkData.value(key.first).erase(key.second) > 0;
  if (resourceLinkData.empty())
  {
    resourceLinkData.erase(key.first);
  }
  return returnValue;
}

bool Links::removeLinksTo(Resource* lhs1, const smtk::common::UUID&, const smtk::common::UUID& rhs1,
  const smtk::common::UUID& rhs2, const RoleType& role)
{
  // Access the Resource Link data that connects this component's resource to
  // the input resource. If it doesn't exist, then there is no link.
  typedef Resource::Links::ResourceLinkData ResourceLinkData;
  ResourceLinkData& resourceLinkData = lhs1->links().data();
  auto& resourceLinks = resourceLinkData.get<ResourceLinkData::Right>();

  // All resource links held by a resource have a lhs = the containing resource.
  // We therefore only need to find the resource link with a rhs = the input
  // parameter resource.
  auto resourceRange = resourceLinks.equal_range(rhs1);

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
  Component::Links::Data& componentLinkData = resourceLinkData.value(resourceRange.first->id);

  return componentLinkData.erase_all<Component::Links::Data::Right>(std::make_tuple(rhs2, role));
}

std::pair<PersistentObjectPtr, Links::RoleType> Links::linkedObjectAndRole(
  const Resource* lhs1, const Links::Key& key) const
{
  typedef Resource::Links::ResourceLinkData ResourceLinkData;
  const ResourceLinkData& resourceLinkData = lhs1->links().data();
  if (resourceLinkData.has(key.first) == false)
  {
    return std::make_pair(ResourcePtr(), Component::Links::Data::undefinedRole);
  }

  auto& resourceLink = resourceLinkData.value(key.first);
  auto& componentLink = resourceLink.at(key.second);

  // Refresh the link using the manager, if one is available
  //
  // TODO: This should be removed once we have accounted for all of the
  // shared pointers to resoruces
  // if (resourceLink.resource() == nullptr && lhs1->manager() != nullptr)
  if (lhs1->manager() != nullptr)
  {
    resourceLink.fetch(lhs1->manager());
  }

  if (componentLink.right == linkToResource)
  {
    return std::make_pair(resourceLink.resource(), componentLink.role);
  }
  else
  {
    if (resourceLink.resource() == nullptr)
    {
      return std::make_pair(ComponentPtr(), componentLink.role);
    }

    return std::make_pair(resourceLink.resource()->find(componentLink.right), componentLink.role);
  }
}

} // namespace resource
} // namespace smtk
