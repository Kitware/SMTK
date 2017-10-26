//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_StructuredGrid_h
#define __smtk_mesh_StructuredGrid_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include "smtk/mesh/MeshSet.h"

#include <functional>
#include <limits>

namespace smtk
{
namespace mesh
{

/**\brief A wrapper for structured data.

   This class is a facade for describing external two-dimensional data sets that
   consist of a structured grid of points and associated scalar values. The user
   must assign an index extent, origin and spacing, as well as an I^2->R
   function describing the scalar value associated with index (i,j).
   Additionally, an I^2->bool function can be passed to the class to denote cell
   validity, facilitating blanking.
  */
class StructuredGrid
{
public:
  StructuredGrid()
    : m_data([](int, int) { return std::numeric_limits<double>::quiet_NaN(); })
    , m_valid([](int, int) { return false; })
  {
    for (int i = 0; i < 4; i++)
    {
      m_extent[i] = 0;
    }
    for (int i = 0; i < 2; i++)
    {
      m_origin[i] = m_spacing[i] = std::numeric_limits<double>::quiet_NaN();
    }
  }

  StructuredGrid(const int extent[4], const double origin[2], const double spacing[2],
    const std::function<double(int, int)>& data, const std::function<bool(int, int)>& valid)
    : m_data(data)
    , m_valid(valid)
  {
    for (int i = 0; i < 4; i++)
    {
      m_extent[i] = extent[i];
    }
    for (int i = 0; i < 2; i++)
    {
      m_origin[i] = origin[i];
      m_spacing[i] = spacing[i];
    }
  }

  StructuredGrid(const int extent[4], const double origin[2], const double spacing[2],
    const std::function<double(int, int)>& data)
    : StructuredGrid(extent, origin, spacing, data, [](int, int) { return true; })
  {
  }

  // Given indices int othe structured data, determine whether or not the cell
  // is valid.
  bool containsIndex(int ix, int iy) const
  {
    return (ix >= m_extent[0] && ix <= m_extent[1] && iy >= m_extent[2] && iy <= m_extent[3]) &&
      m_valid(ix, iy);
  }

  const std::function<double(int, int)>& data() const { return m_data; }

  std::size_t size() const { return (m_extent[1] - m_extent[0]) * (m_extent[3] - m_extent[2]); }

  int m_extent[4];     // [istart, iend, jstart, jend]
  double m_origin[2];  // location of pixel index (0,0)
  double m_spacing[2]; // i, j pixel spacing

private:
  std::function<double(int, int)> m_data;
  std::function<bool(int, int)> m_valid;
};
}
}

#endif
