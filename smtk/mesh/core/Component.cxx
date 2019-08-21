//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/core/Component.h"

#include "smtk/common/UUIDGenerator.h"

#include "smtk/mesh/core/Resource.h"

namespace smtk
{
namespace mesh
{

Component::Component(const smtk::mesh::ResourcePtr& resource, const smtk::common::UUID& id)
  : m_resource(resource)
  , m_id(id)
{
}

Component::Component(const smtk::mesh::MeshSet& meshset)
  : Component(meshset.resource(), meshset.id())
{
}

bool Component::operator==(const Component& other) const
{
  return mesh() == other.mesh();
}

bool Component::operator!=(const Component& other) const
{
  return mesh() != other.mesh();
}

bool Component::operator<(const Component& other) const
{
  return mesh() < other.mesh();
}

std::shared_ptr<Component> Component::create(
  const smtk::mesh::ResourcePtr& resource, const smtk::common::UUID& id)
{
  // If the mesh resource is invalid, return an invalid component
  if (resource == nullptr)
  {
    return std::shared_ptr<Component>();
  }

  // Attempt to find a preexisting component associated with the incident id
  auto idAndComponent = resource->m_componentMap.find(id);
  if (idAndComponent != resource->m_componentMap.end())
  {
    // If one is found, return the stored component
    return idAndComponent->second;
  }

  // If no preexisting component exists, we need to create one.

  // First, confirm there is a meshset associated with the incident id
  smtk::mesh::MeshSet meshset;
  {
    const smtk::mesh::InterfacePtr& iface = resource->interface();
    smtk::mesh::Handle handle;
    smtk::mesh::HandleRange entities;
    if (iface->findById(iface->getRoot(), id, handle) == false)
    {
      // If the id is not associated with a meshset, return an invalid
      // component.
      return std::shared_ptr<Component>();
    }
  }

  // Create a new component to represent the existing meshset
  std::shared_ptr<smtk::resource::Component> shared(new Component(resource, id));

  // Store the component in the resource's component map
  auto ptr = std::static_pointer_cast<smtk::mesh::Component>(shared->shared_from_this());
  resource->m_componentMap[id] = ptr;

  return ptr;
}

std::shared_ptr<Component> Component::create(const smtk::mesh::MeshSet& meshset)
{
  // If the meshset is invalid, return an invalid component
  if (meshset.isValid() == false)
  {
    return std::shared_ptr<Component>();
  }

  // If the meshset has an invalid id, we know it isn't stored in the
  // resource's component map. Generate an id for the meshset and create a new
  // component to represent it.
  if (meshset.id().isNull())
  {
    const_cast<smtk::mesh::MeshSet&>(meshset).setId(
      smtk::common::UUIDGenerator::instance().random());
  }
  else
  {
    // Attempt to find a preexisting component associated with the incident
    // meshset
    auto& resource = meshset.resource();
    auto idAndComponent = resource->m_componentMap.find(meshset.id());
    if (idAndComponent != resource->m_componentMap.end())
    {
      return idAndComponent->second;
    }
  }

  // Create a new component to represent the existing meshset
  std::shared_ptr<smtk::resource::Component> shared(new Component(meshset));

  // Store the component in the resource's component map
  auto ptr = std::static_pointer_cast<smtk::mesh::Component>(shared->shared_from_this());
  meshset.resource()->m_componentMap[meshset.id()] = ptr;

  return ptr;
}

const smtk::resource::ResourcePtr Component::resource() const
{
  return std::static_pointer_cast<smtk::resource::Resource>(m_resource.lock());
}

std::string Component::name() const
{
  auto mesh = this->mesh();
  return (mesh.isValid() ? mesh.name() : "");
}

const smtk::mesh::MeshSet Component::mesh() const
{
  if (auto resource = m_resource.lock())
  {
    const smtk::mesh::InterfacePtr& iface = resource->interface();
    smtk::mesh::Handle handle;
    smtk::mesh::HandleRange entities;
    if (iface->findById(iface->getRoot(), m_id, handle))
    {
      entities.insert(handle);
      return smtk::mesh::MeshSet(resource, iface->getRoot(), entities);
    }
  }
  return smtk::mesh::MeshSet();
}

smtk::mesh::MeshSet Component::mesh()
{
  if (auto resource = m_resource.lock())
  {
    const smtk::mesh::InterfacePtr& iface = resource->interface();
    smtk::mesh::Handle handle;
    smtk::mesh::HandleRange entities;
    if (iface->findById(iface->getRoot(), m_id, handle))
    {
      entities.insert(handle);
      return smtk::mesh::MeshSet(resource, iface->getRoot(), entities);
    }
  }

  return smtk::mesh::MeshSet();
}
}
}
