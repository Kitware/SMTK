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

#include "smtk/mesh/ForEachTypes.h"

#include <limits>

namespace smtk
{
namespace mesh
{

std::array<double, 6> extent(const smtk::mesh::MeshSet& ms)
{
  class Extent : public smtk::mesh::PointForEach
  {
  public:
    Extent()
      : smtk::mesh::PointForEach()
    {
      m_values[0] = m_values[2] = m_values[4] = std::numeric_limits<double>::max();
      m_values[1] = m_values[3] = m_values[5] = std::numeric_limits<double>::lowest();
    }

    void forPoints(const smtk::mesh::HandleRange&, std::vector<double>& xyz, bool&) override
    {
      for (std::size_t i = 0; i < xyz.size(); i += 3)
      {
        for (std::size_t j = 0; j < 3; j++)
        {
          if (xyz[i + j] < this->m_values[2 * j])
          {
            this->m_values[2 * j] = xyz[i + j];
          }
          if (xyz[i + j] > this->m_values[2 * j + 1])
          {
            this->m_values[2 * j + 1] = xyz[i + j];
          }
        }
      }
    }

    std::array<double, 6> m_values;
  };

  Extent extent;
  smtk::mesh::for_each(ms.points(), extent);
  return extent.m_values;
}

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

  // We compute xi by counting the number of adjacency cells at each dimension.
  // Because of the way MOAB deals with higher-order cells, we must treat the
  // vertex count by specifically requesting the number of "corners-only" points
  // rather than the zero-dimensional adjacencies.
  for (int i = highestDim; i >= smtk::mesh::Dims1; i--)
  {
    meshSet = meshSet.extractAdjacenciesOfDimension(i);
    int prefactor = (i % 2 == 0 ? +1 : -1);
    xi += static_cast<long long>(prefactor * meshSet.cells().size());
  }
  xi += static_cast<long long>(meshSet.points(true).size());

  return static_cast<int>(xi);
}
}
}
