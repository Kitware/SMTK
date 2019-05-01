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
  if (m_id == smtk::common::UUID::null())
  {
    m_id = smtk::common::UUIDGenerator::instance().random();
    const_cast<smtk::mesh::MeshSet&>(meshset).setId(m_id);
  }
}

std::shared_ptr<Component> Component::create(
  const smtk::mesh::ResourcePtr& resource, const smtk::common::UUID& id)
{
  std::shared_ptr<smtk::resource::Component> shared(new Component(resource, id));
  return std::static_pointer_cast<smtk::mesh::Component>(shared);
}

std::shared_ptr<Component> Component::create(const smtk::mesh::MeshSet& meshset)
{
  std::shared_ptr<smtk::resource::Component> shared(new Component(meshset));
  return std::static_pointer_cast<smtk::mesh::Component>(shared);
}

const smtk::resource::ResourcePtr Component::resource() const
{
  return std::static_pointer_cast<smtk::resource::Resource>(m_resource.lock());
}

std::string Component::name() const
{
  return this->mesh().name();
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
