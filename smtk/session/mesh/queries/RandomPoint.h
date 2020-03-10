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
  RandomPoint()
    : m_seed(0)
  {
  }

  virtual std::array<double, 3> operator()(const smtk::resource::Component::Ptr& component) const
  {
    if (auto resource =
          std::dynamic_pointer_cast<smtk::session::mesh::Resource>(component->resource()))
    {
      smtk::mesh::Resource::Ptr meshResource = resource->resource();
      auto& randomPoint = meshResource->queries().get<smtk::geometry::RandomPoint>();
      randomPoint.seed(m_seed);
      return randomPoint(
        smtk::mesh::Component::create(meshResource->findAssociatedMeshes(component->id())));
    }

    return smtk::geometry::RandomPoint::operator()(component);
  }

  virtual void seed(std::size_t i) { m_seed = i; }

private:
  std::size_t m_seed;
};
}
}
}

#endif
