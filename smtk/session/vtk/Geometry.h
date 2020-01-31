//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_session_vtk_Geometry_h
#define smtk_session_vtk_Geometry_h

#include "smtk/session/vtk/Exports.h"

#include "smtk/extension/vtk/source/Geometry.h"

#include "smtk/geometry/Cache.h"

#include "smtk/PublicPointerDefs.h"

namespace smtk
{
namespace session
{
namespace vtk
{

class Resource;

/**\brief A VTK geometry provider for the VTK session.
  *
  */
class SMTKVTKSESSION_EXPORT Geometry
  : public smtk::geometry::Cache<smtk::extension::vtk::source::Geometry, Geometry>
{
public:
  using CacheBaseType = smtk::extension::vtk::source::Geometry;
  smtkTypeMacro(smtk::session::vtk::Geometry);
  smtkSuperclassMacro(smtk::geometry::Cache<CacheBaseType, Geometry>);
  using DataType = Superclass::DataType;

  Geometry(const std::shared_ptr<smtk::session::vtk::Resource>& parent);
  virtual ~Geometry() = default;

  smtk::geometry::Resource::Ptr resource() const override;
  void queryGeometry(
    const smtk::resource::PersistentObject::Ptr& obj, CacheEntry& entry) const override;
  int dimension(const smtk::resource::PersistentObject::Ptr& obj) const override;
  Purpose purpose(const smtk::resource::PersistentObject::Ptr& obj) const override;
  void update() const;

  void geometricBounds(const DataType&, BoundingBox& bbox) const;

  static int dimensionOf(const DataType& geom, const smtk::model::EntityPtr& ent);

protected:
  void updateEntity(const smtk::model::EntityPtr&, CacheEntry& entry) const;

  std::weak_ptr<smtk::session::vtk::Resource> m_parent;
};

} // namespace vtk
} // namespace session
} // namespace smtk

#endif // smtk_session_vtk_Geometry_h
