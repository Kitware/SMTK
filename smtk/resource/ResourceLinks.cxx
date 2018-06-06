//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/resource/ResourceLinks.h"

#include "smtk/common/UUID.h"

#include "smtk/resource/Resource.h"

namespace smtk
{
namespace resource
{
ResourceLinks::ResourceLinks(const Resource* resource)
  : lhs(resource)
{
  // NOTE: When modifying this constructor, do not use the resource parameter
  // or lhs field! The parent Resouce is still in construction and is in an
  // indeterminate state.
}

ResourceLinks::~ResourceLinks()
{
}

const ResourceLinks::Link* ResourceLinks::link(const smtk::common::UUID& linkId)
{
  auto search = m_data.find(linkId);
  return search != m_data.end() ? &(*search) : nullptr;
}

bool ResourceLinks::isLinkedTo(const ResourcePtr& resource)
{
  return resource ? m_data.has<ResourceLinkData::Right>(resource->id()) : false;
}

const ResourceLinks::Link* ResourceLinks::addLinkTo(const ResourcePtr& resource)
{
  if (resource == nullptr)
  {
    return nullptr;
  }

  smtk::common::UUID id = smtk::common::UUID::random();

  // Keep spinning uuids until one is accepted.
  while (m_data.has(id))
  {
    id = smtk::common::UUID::random();
  }

  auto inserted =
    m_data.insert(ResourceLinkData::LinkBase(resource), id, lhs->id(), resource->id());

  return &(*inserted.first);
}

bool ResourceLinks::removeLink(const smtk::common::UUID& linkId)
{
  return m_data.erase(linkId) > 0;
}

std::set<std::reference_wrapper<const smtk::common::UUID>, std::less<const smtk::common::UUID> >
ResourceLinks::linkIds() const
{
  return m_data.linked_to<ResourceLinkData::Left>(lhs->id());
}

} // namespace resource
} // namespace smtk
