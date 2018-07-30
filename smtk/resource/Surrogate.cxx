//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/resource/Surrogate.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

#include <cassert>

namespace smtk
{
namespace resource
{

Surrogate::Surrogate(const Index& index, const std::string& typeName, const smtk::common::UUID& id,
  const std::string& location)
  : m_index(index)
  , m_typeName(typeName)
  , m_id(id)
  , m_location(location)
  , m_resource()
{
}

Surrogate::Surrogate(const ResourcePtr& resource)
  : m_index(resource->index())
  , m_typeName(resource->typeName())
  , m_id(resource->id())
  , m_location(resource->location())
  , m_resource(resource)
{
}

ResourcePtr Surrogate::resource() const
{
  return m_resource.lock();
}

ComponentPtr Surrogate::find(const smtk::common::UUID& id) const
{
  if (auto rsrc = m_resource.lock())
  {
    return rsrc->find(id);
  }

  return ComponentPtr();
}

bool Surrogate::fetch(const ManagerPtr& manager) const
{
  // If the input manager is valid...
  if (manager != nullptr)
  {
    // ...check if it is managing the resource to which we link.
    auto resource = manager->get(m_id);

    // If it is, resolve this link with the resource. Otherwise, attempt to read
    // in the resource and resolve the link using the result.
    return (resource != nullptr ? this->resolve(resource)
                                : this->resolve(manager->read(m_index, m_location)));
  }

  // If the manager is invalid, there's not much we can do.
  return false;
}

bool Surrogate::resolve(const ResourcePtr& resource) const
{
  // If the resource is invalid, there's not much we can do.
  if (resource == nullptr)
  {
    return false;
  }

  // For now, let's only check that the type name and id are equivalent. The
  // type index check should be equivalent to the type name check, and resource
  // locations may yet be allowed to change.
  if (m_typeName == resource->typeName() && m_id == resource->id())
  {
    m_resource = resource;
    return true;
  }
  return false;
}

} // namespace resource
} // namespace smtk
