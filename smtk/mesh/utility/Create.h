//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_utility_Create_h
#define __smtk_mesh_utility_Create_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/mesh/core/MeshSet.h"
#include "smtk/mesh/core/Resource.h"

#include <array>

namespace smtk
{
namespace mesh
{
namespace utility
{

// construct a 3-dimensional hexahedral grid discretized along each principal
// axis according to <discretization> and with points transformed from the unit
// cube (0, 0, 0) to (1, 1, 1) according to an R^3->R^3 mapping <transform>.
// Return the meshsets corresponding to volume, x-min, y-min, z-min, x-max,
// y-max, z-max, in that order.
SMTKCORE_EXPORT
std::array<smtk::mesh::MeshSet, 7> createUniformGrid(
  smtk::mesh::ResourcePtr,
  const std::array<std::size_t, 3>& discretization,
  const std::function<std::array<double, 3>(std::array<double, 3>)>& transform);

// construct a 2-dimensional quadrilateral grid discretized along each principal
// axis according to <discretization> and with points transformed from the unit
// square (0, 0, 0) to (1, 1, 0) according to an R^3->R^3 mapping <transform>.
// Return the meshsets corresponding to area, x-min, y-min, x-max, y-max, in
// that order.
SMTKCORE_EXPORT
std::array<smtk::mesh::MeshSet, 5> createUniformGrid(
  smtk::mesh::ResourcePtr,
  const std::array<std::size_t, 2>& discretization,
  const std::function<std::array<double, 3>(std::array<double, 3>)>& transform);
} // namespace utility
} // namespace mesh
} // namespace smtk

#endif
