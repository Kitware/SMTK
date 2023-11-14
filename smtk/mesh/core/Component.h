//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_mesh_Component_h
#define smtk_mesh_Component_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/TupleTraits.h"

#include "smtk/resource/Component.h"

namespace smtk
{
namespace mesh
{
class Component;
class MeshSet;
class Resource;

typedef std::vector<smtk::mesh::Component> ComponentList;
typedef std::set<smtk::mesh::Component> Components;

/// A lightweight object for representing meshset information as a resource
/// component. This is useful for constructing links between meshsets and other
/// resources/components and for representing meshsets within smtk's attribute
/// system.
class SMTKCORE_EXPORT Component : public smtk::resource::Component
{
protected:
  Component(const smtk::mesh::ResourcePtr&, const smtk::common::UUID&);
  Component(const smtk::mesh::MeshSet&);

public:
  smtkTypeMacro(Component);
  smtkSuperclassMacro(smtk::resource::Component);
  smtkSharedFromThisMacro(smtk::resource::Component);

  // Comparison operators
  bool operator==(const Component&) const;
  bool operator!=(const Component&) const;
  bool operator<(const Component&) const;

  /// Construct a mesh component corresponding to a meshset from the input
  /// resource and id. No checking is performed that the resource has a
  /// meshset with this id; if this is the case, the resolved mesh() will return
  /// a default-constructed (and invalid) mesh.
  static std::shared_ptr<Component> create(
    const smtk::mesh::ResourcePtr&,
    const smtk::common::UUID&);

  /// Construct a mesh component correpsonding to the input meshset.
  static std::shared_ptr<Component> create(const smtk::mesh::MeshSet&);

  /// Access the component's resource.
  const smtk::resource::ResourcePtr resource() const override;

  /// Access the component's id.
  const smtk::common::UUID& id() const override { return m_id; }

  /// Set the component's id. No checking is performed that the resource has a
  /// meshset with this id; if this is the case, the resolved mesh() will return
  /// a default-constructed (and invalid) mesh.
  bool setId(const smtk::common::UUID& myID) override
  {
    m_id = myID;
    return true;
  }

  std::string name() const override;

  /// Access the meshset represented by this component.
  virtual const smtk::mesh::MeshSet mesh() const;
  virtual smtk::mesh::MeshSet mesh();

private:
  std::weak_ptr<smtk::mesh::Resource> m_resource;
  smtk::common::UUID m_id;
};

} // namespace mesh
} // namespace smtk

#endif
