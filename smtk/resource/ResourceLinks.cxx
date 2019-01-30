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
namespace detail
{
ResourceLinks::ResourceLinks(Resource* resource)
  : m_resource(resource)
{
  // NOTE: When modifying this constructor, do not use the resource parameter
  // or m_resource field! The parent Resouce is still in construction and is in
  // an indeterminate state.
}

ResourceLinks::~ResourceLinks()
{
}

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
  for (const Surrogate& surrogate : m_data)
  {
    if (surrogate.typeName() == resource->typeName() && surrogate.id() == resource->id())
    {
      surrogate.resolve(resource);
      return true;
    }
  }
  return false;
}
}
}
}
