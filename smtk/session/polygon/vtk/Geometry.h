//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_session_polygon_vtk_Geometry_h
#define smtk_session_polygon_vtk_Geometry_h

#include "smtk/session/polygon/vtk/vtkPolygonOperationsExtModule.h"

#include "smtk/session/polygon/internal/Config.h"

#include "smtk/extension/vtk/source/Geometry.h"

#include "smtk/geometry/Cache.h"

#include "smtk/PublicPointerDefs.h"

namespace smtk
{
namespace session
{
namespace polygon
{

class Resource;

namespace vtk
{

/**\brief A VTK geometry provider for the polygon session.
  *
  */
class VTKPOLYGONOPERATIONSEXT_EXPORT Geometry
  : public smtk::geometry::Cache<smtk::extension::vtk::source::Geometry, Geometry>
{
public:
  using CacheBaseType = smtk::extension::vtk::source::Geometry;
  smtkTypeMacro(smtk::session::polygon::vtk::Geometry);
  smtkSuperclassMacro(smtk::geometry::Cache<CacheBaseType, Geometry>);
  using DataType = Superclass::DataType;
  using VertexPtr = internal::VertexPtr;
  using EdgePtr = internal::EdgePtr;
  using PolyModel = internal::pmodel*;

  Geometry(const std::shared_ptr<smtk::session::polygon::Resource>& parent);
  virtual ~Geometry() = default;

  smtk::geometry::Resource::Ptr resource() const override;
  void queryGeometry(
    const smtk::resource::PersistentObject::Ptr& obj, CacheEntry& entry) const override;
  int dimension(const smtk::resource::PersistentObject::Ptr& obj) const override;
  Purpose purpose(const smtk::resource::PersistentObject::Ptr& obj) const override;
  void update() const;

  void geometricBounds(const DataType&, BoundingBox& bbox) const;

protected:
  void updateVertex(const PolyModel&, const VertexPtr&, CacheEntry& entry) const;
  void updateEdge(const PolyModel&, const EdgePtr&, CacheEntry& entry) const;
  void updateFace(const PolyModel&, const smtk::model::EntityPtr&, CacheEntry& entry) const;

  std::weak_ptr<smtk::session::polygon::Resource> m_parent;
};

} // namespace vtk
} // namespace polygon
} // namespace session
} // namespace smtk

#endif // smtk_session_polygon_vtk_Geometry_h
