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

class StructuredGrid
{
public:
#ifndef SHIBOKEN_SKIP
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
#endif

  // Given indices int othe structured data, determine whether or not the cell
  // is valid.
  bool containsIndex(int ix, int iy) const
  {
    return (ix >= m_extent[0] && ix <= m_extent[1] && iy >= m_extent[2] && iy <= m_extent[3]) &&
      m_valid(ix, iy);
  }

#ifndef SHIBOKEN_SKIP
  const std::function<double(int, int)>& data() const { return m_data; }
#endif

  int m_extent[4];     // [istart, iend, jstart, jend]
  double m_origin[2];  // location of pixel index (0,0)
  double m_spacing[2]; // i, j pixel spacing

private:
  const std::function<double(int, int)> m_data;
  const std::function<bool(int, int)> m_valid;
};
}
}

#endif
