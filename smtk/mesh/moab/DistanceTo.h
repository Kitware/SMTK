//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_mesh_moab_DistanceTo_h
#define smtk_mesh_moab_DistanceTo_h

#include "smtk/CoreExports.h"

#include "smtk/geometry/queries/DistanceTo.h"

#include "smtk/mesh/core/MeshSet.h"

#include "smtk/resource/Component.h"

#include <array>
#include <utility>

namespace smtk
{
namespace mesh
{
namespace moab
{

/**\brief An API for computing the shortest distance between an input point and
  * a geometric resource component. The location of the point on the component
  * is also returned. This query differs from ClosestPoint in that the returned
  * point does not need to be explicitly contained within the geometric
  * representation.
  */
struct SMTKCORE_EXPORT DistanceTo
  : public smtk::resource::query::DerivedFrom<DistanceTo, smtk::geometry::DistanceTo>
{
  std::pair<double, std::array<double, 3> > operator()(
    const smtk::resource::Component::Ptr&, const std::array<double, 3>&) const override;

  std::pair<double, std::array<double, 3> > operator()(
    const smtk::mesh::MeshSet&, const std::array<double, 3>&) const;
};
}
}
}

#endif
