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

Component::Component(const smtk::mesh::CollectionPtr& collection, const smtk::common::UUID& id)
  : m_collection(collection)
  , m_id(id)
{
}

Component::Component(const smtk::mesh::MeshSet& meshset)
  : Component(meshset.collection(), meshset.id())
{
  if (m_id == smtk::common::UUID::null())
  {
    m_id = smtk::common::UUIDGenerator::instance().random();
    const_cast<smtk::mesh::MeshSet&>(meshset).setId(m_id);
  }
}

std::shared_ptr<Component> Component::create(
  const smtk::mesh::CollectionPtr& collection, const smtk::common::UUID& id)
{
  std::shared_ptr<smtk::resource::Component> shared(new Component(collection, id));
  return std::static_pointer_cast<smtk::mesh::Component>(shared);
}

std::shared_ptr<Component> Component::create(const smtk::mesh::MeshSet& meshset)
{
  std::shared_ptr<smtk::resource::Component> shared(new Component(meshset));
  return std::static_pointer_cast<smtk::mesh::Component>(shared);
}

const smtk::resource::ResourcePtr Component::resource() const
{
  return std::static_pointer_cast<smtk::resource::Resource>(m_collection.lock());
}

std::string Component::name() const
{
  return this->mesh().name();
}

const smtk::mesh::MeshSet Component::mesh() const
{
  if (auto collection = m_collection.lock())
  {
    const smtk::mesh::InterfacePtr& iface = collection->interface();
    smtk::mesh::Handle handle;
    if (iface->findById(iface->getRoot(), m_id, handle))
    {
      return smtk::mesh::MeshSet(collection, handle);
    }
  }

  return smtk::mesh::MeshSet();
}

smtk::mesh::MeshSet Component::mesh()
{
  if (auto collection = m_collection.lock())
  {
    const smtk::mesh::InterfacePtr& iface = collection->interface();
    smtk::mesh::Handle handle;
    if (iface->findById(iface->getRoot(), m_id, handle))
    {
      return smtk::mesh::MeshSet(collection, handle);
    }
  }

  return smtk::mesh::MeshSet();
}
}
}
