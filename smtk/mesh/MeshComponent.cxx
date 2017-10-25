//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/MeshComponent.h"

#include "smtk/mesh/Collection.h"

namespace smtk
{
namespace mesh
{

MeshComponent::MeshComponent(smtk::mesh::MeshSet meshset)
  : smtk::resource::Component()
  , m_meshset(meshset)
{
}

MeshComponent::MeshComponent(const smtk::common::UUID& id, smtk::mesh::MeshSet meshset)
  : smtk::resource::Component(id)
  , m_meshset(meshset)
{
}

const smtk::resource::ResourcePtr MeshComponent::resource() const
{
  return std::dynamic_pointer_cast<smtk::resource::Resource>(this->m_meshset.collection());
}

/**\brief Return an array of model entity UUIDs associated with meshset members.
  *
  */
smtk::common::UUIDArray MeshComponent::modelEntityIds() const
{
  if (!this->m_meshset.collection())
  {
    return smtk::common::UUIDArray();
  }

  const smtk::mesh::InterfacePtr& iface = this->m_meshset.collection()->interface();
  return iface->computeModelEntities(this->m_meshset.range());
}

/**\brief Return the model entities associated with meshset members.
  *
  * warning Note that the parent collection of the meshset must have
  *         its model manager set to a valid value or the result will
  *         be an array of invalid entries.
  */
bool MeshComponent::modelEntities(smtk::model::EntityRefArray& array) const
{
  if (!this->m_meshset.collection())
  {
    return false;
  }

  smtk::model::ManagerPtr mgr = this->m_meshset.collection()->modelManager();
  smtk::common::UUIDArray uids = this->modelEntityIds();
  for (smtk::common::UUIDArray::const_iterator it = uids.begin(); it != uids.end(); ++it)
  {
    array.push_back(smtk::model::EntityRef(mgr, *it));
  }
  return (mgr != nullptr);
}

/**\brief Set the model entity for each meshset member to \a ent.
  *
  */
bool MeshComponent::setModelEntity(const smtk::model::EntityRef& ent)
{
  if (!this->m_meshset.collection())
  {
    return false;
  }

  const smtk::mesh::InterfacePtr& iface = this->m_meshset.collection()->interface();
  return iface->setAssociation(ent.entity(), this->m_meshset.range());
}

/**\brief Set the model entity for each meshset member to \a ent.
  *
  */
bool MeshComponent::setModelEntityId(const smtk::common::UUID& id)
{
  if (!this->m_meshset.collection())
  {
    return false;
  }

  const smtk::mesh::InterfacePtr& iface = this->m_meshset.collection()->interface();
  return iface->setAssociation(id, this->m_meshset.range());
}
}
}
