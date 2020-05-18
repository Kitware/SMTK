//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_vtk_mesh_Geometry_h
#define smtk_extension_vtk_mesh_Geometry_h

#include "smtk/extension/vtk/geometry/Geometry.h"
#include "smtk/extension/vtk/mesh/vtkSMTKMeshExtModule.h"

#include "smtk/geometry/Cache.h"

#include "smtk/PublicPointerDefs.h"

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace mesh
{

/**\brief A VTK geometry provider for smtk mesh resources.
  *
  */
class VTKSMTKMESHEXT_EXPORT Geometry
  : public smtk::geometry::Cache<smtk::extension::vtk::geometry::Geometry>
{
public:
  using CacheBaseType = smtk::extension::vtk::geometry::Geometry;
  smtkTypeMacro(smtk::extension::vtk::geometry::Geometry);
  smtkSuperclassMacro(smtk::geometry::Cache<CacheBaseType>);
  using DataType = Superclass::DataType;

  Geometry(const std::shared_ptr<smtk::mesh::Resource>& parent);
  virtual ~Geometry() = default;

  smtk::geometry::Resource::Ptr resource() const override;
  void queryGeometry(const smtk::resource::PersistentObject::Ptr& obj, CacheEntry& entry)
    const override;
  int dimension(const smtk::resource::PersistentObject::Ptr& obj) const override;
  Purpose purpose(const smtk::resource::PersistentObject::Ptr& obj) const override;
  void update() const override;

  void geometricBounds(const DataType&, BoundingBox& bbox) const override;

protected:
  std::weak_ptr<smtk::mesh::Resource> m_parent;
};
} // namespace mesh
} // namespace vtk
} // namespace extension
} // namespace smtk

#endif
