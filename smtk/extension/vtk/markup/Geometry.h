//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_vtk_markup_Geometry_h
#define smtk_extension_vtk_markup_Geometry_h

#include "smtk/extension/vtk/geometry/Geometry.h"
#include "smtk/extension/vtk/markup/vtkSMTKMarkupExtModule.h"

#include "smtk/geometry/Cache.h"

#include "smtk/PublicPointerDefs.h"

namespace smtk
{
namespace markup
{
class Resource;
}
namespace extension
{
namespace vtk
{
namespace markup
{

/**\brief A VTK geometry provider for smtk markup resources.
  *
  */
class VTKSMTKMARKUPEXT_EXPORT Geometry
  : public smtk::geometry::Cache<smtk::extension::vtk::geometry::Geometry>
{
public:
  using CacheBaseType = smtk::extension::vtk::geometry::Geometry;
  smtkTypeMacro(smtk::extension::vtk::geometry::Geometry);
  smtkSuperclassMacro(smtk::geometry::Cache<CacheBaseType>);
  using DataType = Superclass::DataType;

  Geometry(const std::shared_ptr<smtk::markup::Resource>& parent);
  ~Geometry() override = default;

  smtk::geometry::Resource::Ptr resource() const override;
  void queryGeometry(const smtk::resource::PersistentObject::Ptr& obj, CacheEntry& entry)
    const override;
  int dimension(const smtk::resource::PersistentObject::Ptr& obj) const override;
  Purpose purpose(const smtk::resource::PersistentObject::Ptr& obj) const override;
  void update() const override;

  void geometricBounds(const DataType&, BoundingBox& bbox) const override;

protected:
  std::weak_ptr<smtk::markup::Resource> m_parent;
};
} // namespace markup
} // namespace vtk
} // namespace extension
} // namespace smtk

#endif
