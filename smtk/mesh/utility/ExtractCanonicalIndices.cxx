//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/utility/ExtractCanonicalIndices.h"

#include "smtk/mesh/core/CellSet.h"
#include "smtk/mesh/core/CellTraits.h"
#include "smtk/mesh/core/Handle.h"
#include "smtk/mesh/core/Interface.h"
#include "smtk/mesh/core/MeshSet.h"
#include "smtk/mesh/core/Resource.h"

#include "smtk/mesh/utility/Metrics.h"

#include <cmath>
#include <cstring>
#include <limits>
#include <unordered_map>
#include <utility>

namespace smtk
{
namespace mesh
{
namespace utility
{

void PreAllocatedCanonicalIndices::determineAllocationLengths(
  const smtk::mesh::MeshSet& ms,
  std::int64_t& numberOfCells)
{
  numberOfCells = ms.cells().size();
}

PreAllocatedCanonicalIndices::PreAllocatedCanonicalIndices(
  std::int64_t* referenceCellIndices,
  std::int64_t* canonicalIndices)
  : m_referenceCellIndices(referenceCellIndices)
  , m_canonicalIndices(canonicalIndices)
{
}

void CanonicalIndices::extract(
  const smtk::mesh::MeshSet& ms,
  const smtk::mesh::MeshSet& referenceMS)
{
  //determine the lengths
  std::int64_t numberOfCells = -1;

  PreAllocatedCanonicalIndices::determineAllocationLengths(ms, numberOfCells);
  m_referenceCellIndices.resize(numberOfCells);
  m_canonicalIndices.resize(numberOfCells);

  PreAllocatedCanonicalIndices field(m_referenceCellIndices.data(), m_canonicalIndices.data());

  extractCanonicalIndices(ms, referenceMS, field);
}

void extractCanonicalIndices(
  const smtk::mesh::MeshSet& ms,
  const smtk::mesh::MeshSet& referenceMS,
  PreAllocatedCanonicalIndices& field)
{
  // Check that the mesh sets come from the same resource.
  if (ms.resource() != referenceMS.resource())
  {
    return;
  }

  // Construct a map between the cell handles in <referenceMS> that are adjacent
  // to the cells in <ms> and their indices.
  std::unordered_map<std::int64_t, std::size_t> cellMap;

  {
    // <referenceMS> may have a lot of cells. We don't need to add each one to
    // our cell map, though. We only need the ones that are reference cells for
    // the cells in <ms>. To determine which cells to use, we first extract the
    // higher-dimension adjacencies of <ms>.
    int dimension = static_cast<int>(smtk::mesh::utility::highestDimension(ms)) + 1;

    bool adjacencyMeshCreated = false;
    smtk::mesh::MeshSet adjacencyMesh =
      ms.extractAdjacenciesOfDimension(dimension, adjacencyMeshCreated);
    smtk::mesh::CellSet adjacencies = adjacencyMesh.cells();

    // We then iterate over each of our reference cells, incrementing a counter
    // as we go.
    smtk::mesh::CellSet referenceCells = referenceMS.cells();
    auto it = rangeElementsBegin(referenceCells.range());
    auto end = rangeElementsEnd(referenceCells.range());
    for (std::size_t counter = 0; it != end; ++it, ++counter)
    {
      // If the cell is an adjacency of <ms>, add it to the map.
      if (rangeContains(adjacencies.range(), *it))
      {
        cellMap[*it] = counter;
      }
    }

    if (adjacencyMeshCreated)
    {
      ms.resource()->removeMeshes(adjacencyMesh);
    }
  }

  // Now that our cell map is constructed, we iterate over the cells in <ms> and
  // identify the canonical index for each cell.
  auto interface = ms.resource()->interface();
  smtk::mesh::CellSet cells = ms.cells();
  auto it = rangeElementsBegin(cells.range());
  auto end = rangeElementsEnd(cells.range());

  smtk::mesh::Handle parent;
  int canonicalIndex;
  for (std::size_t counter = 0; it != end; ++it, ++counter)
  {
    interface->canonicalIndex(*it, parent, canonicalIndex);
    field.m_referenceCellIndices[counter] = cellMap[parent];
    field.m_canonicalIndices[counter] = static_cast<std::size_t>(canonicalIndex);
  }
}
} // namespace utility
} // namespace mesh
} // namespace smtk
