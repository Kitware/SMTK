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

#include <algorithm>

namespace smtk
{
namespace resource
{
namespace detail
{
ResourceLinks::ResourceLinks(Resource* resource)
  : m_resource(resource)
{
  // NOTE: When modifying this constructor, do not use the resource parameter
  // or m_resource field! The parent Resouce is still in construction and is in
  // an indeterminate state.
}

ResourceLinks::~ResourceLinks() = default;

Resource* ResourceLinks::leftHandSideResource()
{
  return m_resource;
}

const Resource* ResourceLinks::leftHandSideResource() const
{
  return m_resource;
}

bool ResourceLinks::resolve(const ResourcePtr& resource) const
{
  auto i = std::find_if(m_data.begin(), m_data.end(), [&resource](const Surrogate& surrogate) {
    return surrogate.typeName() == resource->typeName() && surrogate.id() == resource->id();
  });
  if (i == m_data.end())
  {
    return false;
  }
  i->resolve(resource);
  return true;
}

bool ResourceLinks::removeAllLinksTo(const ResourcePtr& resource)
{
  return this->data().erase_all<ResourceLinkData::Right>(resource->id());
}
} // namespace detail
} // namespace resource
} // namespace smtk
