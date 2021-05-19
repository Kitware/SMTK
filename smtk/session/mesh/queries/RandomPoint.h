//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_session_mesh_RandomPoint_h
#define smtk_session_mesh_RandomPoint_h

#include "smtk/session/mesh/Exports.h"

#include "smtk/geometry/queries/RandomPoint.h"

#include "smtk/mesh/core/Component.h"

#include "smtk/session/mesh/Resource.h"

namespace smtk
{
namespace session
{
namespace mesh
{

/**\brief An API for computing a random point on a geometric resource component.
  */
struct SMTKMESHSESSION_EXPORT RandomPoint
  : public smtk::resource::query::DerivedFrom<RandomPoint, smtk::geometry::RandomPoint>
{
  RandomPoint() = default;

  std::array<double, 3> operator()(const smtk::resource::Component::Ptr& component) const override
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
        return meshResource->queries().get<smtk::geometry::RandomPoint>().operator()(
          smtk::mesh::Component::create(elementIt->second.m_mesh));
      }
    }

    return smtk::geometry::RandomPoint::operator()(component);
  }

  void seed(std::size_t i) override { m_seed = i; }

private:
  std::size_t m_seed{ 0 };
};
} // namespace mesh
} // namespace session
} // namespace smtk

#endif
