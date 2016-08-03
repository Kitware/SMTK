//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_common_GeometryUtilities_h
#define __smtk_common_GeometryUtilities_h

namespace smtk {
  namespace common {

/// Return the square of the Euclidean distance between \a p0 and \a p1.
template<typename T>
double distance2(const T* p0, const T* p1)
{
  double d2 = 0.;
  for (int i = 0; i < 3; ++i)
    {
    double delta = p1[i] - p0[i];
    d2 += delta * delta;
    }
  return d2;
}

  } // namespace common
} // namespace smtk

#endif // __smtk_common_GeometryUtilities_h
