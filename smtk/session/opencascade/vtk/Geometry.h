//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_session_opencascade_vtk_Geometry_h
#define smtk_session_opencascade_vtk_Geometry_h

#include "smtk/session/opencascade/vtk/vtkOpencascadeGeometryExtModule.h"

#include "smtk/extension/vtk/geometry/Geometry.h"

#include "smtk/geometry/Cache.h"

#include "smtk/PublicPointerDefs.h"

class TopoDS_Shape;
class TopoDS_Vertex;
class TopoDS_Edge;
class TopoDS_Face;
class TopoDS_Solid;

namespace smtk
{
namespace session
{
namespace opencascade
{

class Resource;

namespace vtk
{

/**\brief A VTK geometry provider for the opencascade session.
  *
  */
class VTKOPENCASCADEGEOMETRYEXT_EXPORT Geometry
  : public smtk::geometry::Cache<smtk::extension::vtk::geometry::Geometry, Geometry>
{
public:
  using CacheBaseType = smtk::extension::vtk::geometry::Geometry;
  smtkTypeMacro(smtk::session::opencascade::vtk::Geometry);
  smtkSuperclassMacro(smtk::geometry::Cache<CacheBaseType, Geometry>);
  using DataType = Superclass::DataType;
  using Shape = TopoDS_Shape;
  using Vertex = TopoDS_Vertex;
  using Edge = TopoDS_Edge;
  using Face = TopoDS_Face;
  using Volume = TopoDS_Solid;

  Geometry(const std::shared_ptr<smtk::session::opencascade::Resource>& parent);
  virtual ~Geometry() = default;

  smtk::geometry::Resource::Ptr resource() const override;
  void queryGeometry(
    const smtk::resource::PersistentObject::Ptr& obj, CacheEntry& entry) const override;
  int dimension(const smtk::resource::PersistentObject::Ptr& obj) const override;
  Purpose purpose(const smtk::resource::PersistentObject::Ptr& obj) const override;
  void update() const;

  void geometricBounds(const DataType&, BoundingBox& bbox) const;

protected:
  void updateVertex(const Vertex& face, CacheEntry& entry) const;
  void updateEdge(const Edge& face, CacheEntry& entry) const;
  void updateFace(const Face& face, CacheEntry& entry) const;

  std::weak_ptr<smtk::session::opencascade::Resource> m_parent;
};

} // namespace vtk
} // namespace opencascade
} // namespace session
} // namespace smtk

#endif // smtk_session_opencascade_vtk_Geometry_h
