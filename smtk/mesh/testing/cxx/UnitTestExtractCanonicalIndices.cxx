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

#include "smtk/common/UUID.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/io/ModelToMesh.h"
#include "smtk/io/ReadMesh.h"

#include "smtk/mesh/core/CellField.h"
#include "smtk/mesh/core/Component.h"
#include "smtk/mesh/core/ForEachTypes.h"
#include "smtk/mesh/core/Resource.h"
#include "smtk/mesh/utility/Create.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <array>
#include <functional>

int UnitTestExtractCanonicalIndices(int /*unused*/, char* /*unused*/[])
{
  // Create a new mesh mesh resource
  smtk::mesh::ResourcePtr meshResource = smtk::mesh::Resource::create();

  // Construct a uniform grid
  std::array<std::size_t, 3> discretization = { { 2, 2, 2 } };
  std::function<std::array<double, 3>(std::array<double, 3>)> transform =
    [](std::array<double, 3> x) { return x; };
  auto meshes = smtk::mesh::utility::createUniformGrid(meshResource, discretization, transform);

  std::array<std::int64_t, 24> validReferenceCellIndices = { 0, 1, 2, 3, 0, 1, 4, 5, 0, 2, 4, 6,
                                                             4, 5, 6, 7, 2, 3, 6, 7, 1, 3, 5, 7 };

  std::array<std::int64_t, 24> validCanonicalIndices = { 4, 4, 4, 4, 0, 0, 0, 0, 3, 3, 3, 3,
                                                         5, 5, 5, 5, 2, 2, 2, 2, 1, 1, 1, 1 };

  // Loop over the surface meshsets
  for (int i = 1; i < 7; i++)
  {
    // Construct the arrays to hold reference cell indices and canonical indices
    std::int64_t numberOfCells;
    smtk::mesh::utility::PreAllocatedCanonicalIndices::determineAllocationLengths(
      meshes[i], numberOfCells);
    std::vector<std::int64_t> referenceCellIndices(numberOfCells);
    std::vector<std::int64_t> canonicalIndices(numberOfCells);

    smtk::mesh::utility::PreAllocatedCanonicalIndices indices(
      referenceCellIndices.data(), canonicalIndices.data());

    smtk::mesh::utility::extractCanonicalIndices(meshes[i], meshes[0], indices);

    smtkTest(
      std::equal(
        referenceCellIndices.begin(),
        referenceCellIndices.end(),
        validReferenceCellIndices.begin() + (i - 1) * 4),
      "Incorrect reference cell indices.");
    smtkTest(
      std::equal(
        canonicalIndices.begin(),
        canonicalIndices.end(),
        validCanonicalIndices.begin() + (i - 1) * 4),
      "Invalid canonical indices.");
  }

  return 0;
}
