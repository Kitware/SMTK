//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_mesh_moab_RandomPoint_h
#define smtk_mesh_moab_RandomPoint_h

#include "smtk/CoreExports.h"

#include "smtk/geometry/queries/RandomPoint.h"

#include "smtk/mesh/core/MeshSet.h"

#include "smtk/resource/Component.h"

#include <array>

namespace smtk
{
namespace mesh
{
namespace moab
{

/**\brief An API for computing a random point on a geometric resource component.
  */
struct SMTKCORE_EXPORT RandomPoint
  : public smtk::resource::query::DerivedFrom<RandomPoint, smtk::geometry::RandomPoint>
{
  RandomPoint() = default;

  std::array<double, 3> operator()(const smtk::resource::Component::Ptr&) const override;

  std::array<double, 3> operator()(const smtk::mesh::MeshSet&) const;

  void seed(std::size_t seed) override { m_seed = seed; }

private:
  std::size_t m_seed{ 0 };
};
} // namespace moab
} // namespace mesh
} // namespace smtk

#endif
