//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_utility_Metrics_h
#define __smtk_mesh_utility_Metrics_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/mesh/core/DimensionTypes.h"
#include "smtk/mesh/core/MeshSet.h"

#include <array>
#include <string>

namespace smtk
{
namespace mesh
{
namespace utility
{

// Compute the bounding box of a mesh set
SMTKCORE_EXPORT
std::array<double, 6> extent(const smtk::mesh::MeshSet& ms);

// Compute the highest cell dimension present a mesh set
SMTKCORE_EXPORT
smtk::mesh::DimensionType highestDimension(const smtk::mesh::MeshSet& ms);

// Compute the Euler-Poincare characteristic of a mesh set
SMTKCORE_EXPORT
int eulerCharacteristic(const smtk::mesh::MeshSet& ms);
} // namespace utility
} // namespace mesh
} // namespace smtk

#endif
