//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_mesh_utility_ExtractCanonicalIndices_h
#define smtk_mesh_utility_ExtractCanonicalIndices_h

#include <cstdint>

#include "smtk/mesh/core/CellSet.h"
#include "smtk/mesh/core/MeshSet.h"

namespace smtk
{
namespace mesh
{
namespace utility
{

class SMTKCORE_EXPORT PreAllocatedCanonicalIndices
{

public:
  static void determineAllocationLengths(
    const smtk::mesh::MeshSet& ms,
    std::int64_t& numberOfCells);

  PreAllocatedCanonicalIndices(std::int64_t* referenceCellIndices, std::int64_t* canonicalIndices);

private:
  friend SMTKCORE_EXPORT void extractCanonicalIndices(
    const smtk::mesh::MeshSet&,
    const smtk::mesh::MeshSet&,
    PreAllocatedCanonicalIndices&);

  std::int64_t* m_referenceCellIndices;
  std::int64_t* m_canonicalIndices;
};

// Given a meshset (typically a boundary mesh) and a reference meshset, for each
// cell in the meshset compute the cell's parent index (integral value as
// determined when extracting the reference meshset's tessellation) and the
// canonical index of the cell as defined in Tautges, Timothy J. "Canonical
// numbering systems for finite-element codes." International Journal for
// Numerical Methods in Biomedical Engineering 26.12 (2010): 1559-1572.
class SMTKCORE_EXPORT CanonicalIndices
{
public:
  CanonicalIndices() = default;

  // Assume the cells in <ms> are of the same dimension
  void extract(const smtk::mesh::MeshSet& ms, const smtk::mesh::MeshSet& referenceMS);

  //use these methods to gain access to the fields after extraction
  const std::vector<std::int64_t>& referenceCellIndices() const { return m_referenceCellIndices; }
  const std::vector<std::int64_t>& canonicalIndices() const { return m_canonicalIndices; }

private:
  std::vector<std::int64_t> m_referenceCellIndices;
  std::vector<std::int64_t> m_canonicalIndices;
};

//Don't wrap these for python, instead python should use the CanonicalIndices class and
//the extract method

SMTKCORE_EXPORT void extractCanonicalIndices(
  const smtk::mesh::MeshSet&,
  const smtk::mesh::MeshSet&,
  PreAllocatedCanonicalIndices&);
} // namespace utility
} // namespace mesh
} // namespace smtk

#endif
