//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/Metrics.h"

namespace smtk
{
namespace mesh
{

smtk::mesh::DimensionType highestDimension(const smtk::mesh::MeshSet& ms)
{
  int highestDimension = smtk::mesh::Dims3;
  while (ms.cells(static_cast<smtk::mesh::DimensionType>(highestDimension)).is_empty() &&
    highestDimension >= 0)
  {
    --highestDimension;
  }

  return highestDimension >= 0 ? static_cast<smtk::mesh::DimensionType>(highestDimension)
                               : smtk::mesh::DimensionType_MAX;
}

int eulerCharacteristic(const smtk::mesh::MeshSet& ms)
{
  // We store xi as a long long rather than an int because the algorithm adds
  // and subtracts large integral values during its computation.
  long long xi = 0;
  smtk::mesh::MeshSet meshSet = ms;

  smtk::mesh::DimensionType highestDim = highestDimension(ms);

  if (highestDim == smtk::mesh::DimensionType_MAX)
  {
    return 0;
  }

  for (int i = highestDim; i >= smtk::mesh::Dims0; i--)
  {
    meshSet = meshSet.extractAdjacenciesOfDimension(i);
    int prefactor = (i % 2 == 0 ? +1 : -1);
    xi += static_cast<long long>(prefactor * meshSet.cells().size());
    meshSet.mergeCoincidentContactPoints();
  }

  return static_cast<int>(xi);
}
}
}
