//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_session_mesh_DistanceTo_h
#define smtk_session_mesh_DistanceTo_h

#include "smtk/session/mesh/Exports.h"

#include "smtk/geometry/queries/DistanceTo.h"

#include "smtk/mesh/core/Component.h"

#include "smtk/session/mesh/Resource.h"

namespace smtk
{
namespace session
{
namespace mesh
{
/**\brief An API for computing the shortest distance between an input point and
  * a geometric resource component. The location of the point on the component
  * is also returned. This query differs from ClosestPoint in that the returned
  * point does not need to be explicitly contained within the geometric
  * representation.
  */
struct SMTKMESHSESSION_EXPORT DistanceTo
  : public smtk::resource::query::DerivedFrom<DistanceTo, smtk::geometry::DistanceTo>
{
  std::pair<double, std::array<double, 3>> operator()(
    const smtk::resource::Component::Ptr& component,
    const std::array<double, 3>& sourcePoint) const override
  {
    if (
      auto resource =
        std::dynamic_pointer_cast<smtk::session::mesh::Resource>(component->resource()))
    {
      smtk::session::mesh::Topology* topology = resource->session()->topology(resource);
      auto elementIt = topology->m_elements.find(component->id());

      if (elementIt != topology->m_elements.end())
      {
        smtk::mesh::Resource::Ptr meshResource = resource->resource();
        return meshResource->queries().get<smtk::geometry::DistanceTo>().operator()(
          smtk::mesh::Component::create(elementIt->second.m_mesh), sourcePoint);
      }
    }

    return smtk::geometry::DistanceTo::operator()(component, sourcePoint);
  }
};
} // namespace mesh
} // namespace session
} // namespace smtk

#endif
