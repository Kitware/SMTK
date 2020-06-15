//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_session_mesh_vtk_Geometry_h
#define smtk_session_mesh_vtk_Geometry_h

#include "smtk/session/mesh/Exports.h"

#include "smtk/extension/vtk/geometry/Geometry.h"

#include "smtk/geometry/Cache.h"
#include "smtk/geometry/Generator.h"

#include "smtk/PublicPointerDefs.h"

namespace smtk
{
namespace session
{
namespace mesh
{

class Resource;

namespace vtk
{

/**\brief A VTK geometry provider for the mesh session.
  *
  */
class SMTKMESHSESSION_EXPORT Geometry
  : public smtk::geometry::Cache<smtk::extension::vtk::geometry::Geometry>
{
public:
  using CacheBaseType = smtk::extension::vtk::geometry::Geometry;
  smtkTypeMacro(smtk::session::mesh::vtk::Geometry);
  smtkSuperclassMacro(smtk::geometry::Cache<CacheBaseType>);

  Geometry(const std::shared_ptr<smtk::session::mesh::Resource>& parent);
  virtual ~Geometry() = default;

  smtk::geometry::Resource::Ptr resource() const override;
  void queryGeometry(
    const smtk::resource::PersistentObject::Ptr& obj, CacheEntry& entry) const override;
  int dimension(const smtk::resource::PersistentObject::Ptr& obj) const override;
  Purpose purpose(const smtk::resource::PersistentObject::Ptr& obj) const override;
  void update() const override;

  void geometricBounds(const DataType&, BoundingBox& bbox) const override;

  std::weak_ptr<smtk::session::mesh::Resource> m_parent;
};
}

class SMTKMESHSESSION_EXPORT RegisterVTKBackend
  : public smtk::geometry::Supplier<RegisterVTKBackend>
{
public:
  bool valid(const Specification& in) const override
  {
    smtk::extension::vtk::geometry::Backend backend;
    return std::get<1>(in).index() == backend.index();
  }

  GeometryPtr operator()(const Specification& in) override
  {
    auto rsrc = std::dynamic_pointer_cast<smtk::session::mesh::Resource>(std::get<0>(in));
    if (rsrc)
    {
      auto provider = new vtk::Geometry(rsrc);
      return GeometryPtr(provider);
    }
    throw std::invalid_argument("Not a mesh session resource.");
    return nullptr;
  }
};
}
}
}

#endif
