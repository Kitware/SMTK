//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_MeshComponent_h
#define __smtk_mesh_MeshComponent_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/resource/Component.h"

#include "smtk/mesh/MeshSet.h"

namespace smtk
{
namespace mesh
{
class MeshComponent;

typedef std::vector<smtk::mesh::MeshComponent> MeshComponentList;
typedef std::set<smtk::mesh::MeshComponent> MeshComponents;

class SMTKCORE_EXPORT MeshComponent : public smtk::resource::Component
{
protected:
  MeshComponent(smtk::mesh::MeshSet meshset = smtk::mesh::MeshSet());
  MeshComponent(const smtk::common::UUID& id, smtk::mesh::MeshSet ms = smtk::mesh::MeshSet());

  // Expose all MeshSet constructors
  template <typename... T>
  MeshComponent(T&&... all)
    : MeshComponent(MeshSet(std::forward<T>(all)...))
  {
  }

  template <typename... T>
  MeshComponent(const smtk::common::UUID& id, T&&... all)
    : MeshComponent(id, MeshSet(std::forward<T>(all)...))
  {
  }

public:
  typedef smtk::shared_ptr<MeshComponent> MeshComponentPtr;

  smtkTypeMacro(MeshComponent);
  smtkSharedPtrCreateMacro(smtk::resource::Component);

  // Expose all constructors as static create methods
  template <typename... T>
  static MeshComponentPtr create(T&&... all)
  {
    return MeshComponentPtr(new MeshComponent(std::forward<T>(all)...));
  }

  const smtk::resource::ResourcePtr resource() const override;

  smtk::common::UUIDArray modelEntityIds() const;

  // Append the passed EntityRefArray with the model entities associated with
  // this meshset, and return true on success. If the MeshSet's parent collection
  // does not have its ModelManager set, this method will fail even though
  // modelEntityIds() will still be valid.
  bool modelEntities(smtk::model::EntityRefArray&) const;
  bool setModelEntityId(const smtk::common::UUID&);
  bool setModelEntity(const smtk::model::EntityRef&);

  smtk::common::UUID id() const override { return this->m_meshset.id(); }

  smtk::mesh::MeshSet meshes() const { return this->m_meshset; }

protected:
  void setId(const smtk::common::UUID& id) override { this->m_meshset.setId(id); }

private:
  smtk::mesh::MeshSet m_meshset;
};

} // namespace mesh
} // namespace smtk

#endif
