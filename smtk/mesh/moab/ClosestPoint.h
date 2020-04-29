//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_mesh_moab_ClosestPoint_h
#define smtk_mesh_moab_ClosestPoint_h

#include "smtk/CoreExports.h"

#include "smtk/geometry/queries/ClosestPoint.h"

#include "smtk/mesh/core/MeshSet.h"

#include "smtk/resource/Component.h"

#include <array>

namespace smtk
{
namespace mesh
{
namespace moab
{

/**\brief An API for computing a the closest point on a geometric resource
  * component to an input point. The returned value represents a tessellation or
  * model vertex; no interpolation is performed.
  */
struct SMTKCORE_EXPORT ClosestPoint
  : public smtk::resource::query::DerivedFrom<ClosestPoint, smtk::geometry::ClosestPoint>
{
  std::array<double, 3> operator()(
    const smtk::resource::Component::Ptr&, const std::array<double, 3>&) const override;

  std::array<double, 3> operator()(const smtk::mesh::MeshSet&, const std::array<double, 3>&) const;
};
}
}
}

#endif
