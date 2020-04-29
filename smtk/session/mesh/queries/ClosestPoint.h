//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_session_mesh_ClosestPoint_h
#define smtk_session_mesh_ClosestPoint_h

#include "smtk/session/mesh/Exports.h"

#include "smtk/geometry/queries/ClosestPoint.h"

#include "smtk/mesh/core/Component.h"

#include "smtk/session/mesh/Resource.h"

namespace smtk
{
namespace session
{
namespace mesh
{

/**\brief An API for computing a the closest point on a geometric resource
  * component to an input point. The returned value represents a tessellation or
  * model vertex; no interpolation is performed.
  */
struct SMTKMESHSESSION_EXPORT ClosestPoint
  : public smtk::resource::query::DerivedFrom<ClosestPoint, smtk::geometry::ClosestPoint>
{
  virtual std::array<double, 3> operator()(
    const smtk::resource::Component::Ptr& component, const std::array<double, 3>& sourcePoint) const
  {
    if (auto resource =
          std::dynamic_pointer_cast<smtk::session::mesh::Resource>(component->resource()))
    {
      smtk::mesh::Resource::Ptr meshResource = resource->resource();
      auto& closestPoint = meshResource->queries().get<smtk::geometry::ClosestPoint>();
      return closestPoint(
        smtk::mesh::Component::create(meshResource->findAssociatedMeshes(component->id())),
        sourcePoint);
    }

    return smtk::geometry::ClosestPoint::operator()(component, sourcePoint);
  }
};
}
}
}

#endif
