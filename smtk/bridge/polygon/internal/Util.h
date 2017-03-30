//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#ifndef __smtk_bridge_polygon_internal_Util_h
#define __smtk_bridge_polygon_internal_Util_h

#include "smtk/bridge/polygon/internal/Config.h"

namespace smtk
{
namespace bridge
{
namespace polygon
{

inline internal::HighPrecisionCoord dot2d(const internal::Coord oa[2], const internal::Coord oo[2])
{
  internal::HighPrecisionCoord result;
  result = static_cast<internal::HighPrecisionCoord>(oa[0]) * oo[0] +
    static_cast<internal::HighPrecisionCoord>(oa[1]) * oo[1];
  return result;
}

inline internal::HighPrecisionCoord cross2d(
  const internal::Coord oa[2], const internal::Coord oo[2])
{
  internal::HighPrecisionCoord result;
  result = static_cast<internal::HighPrecisionCoord>(oa[0]) * oo[1] -
    static_cast<internal::HighPrecisionCoord>(oa[1]) * oo[0];
  return result;
}

inline internal::HighPrecisionCoord deltacross2d(const internal::Point& a0,
  const internal::Point& a1, const internal::Point& b0, const internal::Point& b1)
{
  internal::Coord a01[2];
  internal::Coord b01[2];
  a01[0] = a1.x() - a0.x();
  a01[1] = a1.y() - a0.y();
  b01[0] = b1.x() - b0.x();
  b01[1] = b1.y() - b0.y();

  return cross2d(a01, b01);
}

} // namespace polygon
} // namespace bridge
} // namespace smtk

#endif // __smtk_bridge_polygon_internal_Util_h
