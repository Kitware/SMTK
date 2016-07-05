/**
 * MOAB, a Mesh-Oriented datABase, is a software component for creating,
 * storing and accessing finite element mesh data.
 *
 * Copyright 2004 Sandia Corporation.  Under the terms of Contract
 * DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
 * retains certain rights in this software.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 */

#ifndef MB_UTIL_HPP
#define MB_UTIL_HPP

#include "moab/MOABConfig.h"
#include "moab/Forward.hpp"

#include <cmath>
#if defined MOAB_HAVE_ISFINITE
# define moab_isfinite(f) isfinite(f)
//If we have c++11 support, we will have std::isfinite
#elif (defined(__cplusplus) && __cplusplus >= 201103L)
# define moab_isfinite(f) std::isfinite(f)
#elif (defined(_WIN32) && defined(_MSC_VER)) || defined(__MINGW32__)
# include <float.h>
# define moab_isfinite(A) _finite(A)
#elif defined MOAB_HAVE_FINITE
# define moab_isfinite(f) finite(f)
#else
# define moab_isfinite(f) (!isinf(f) && !isnan(f))
#endif

namespace moab {

/** \struct Coord
 * \brief Structure for storing coordinate data
 */
struct  Coord
{
  double x;
  double y;
  double z;
};

/** \class Util
 *
 * \brief Utility functions for normal and centroid for entities
 */
class Util
{
public:

  static void normal(Interface* MB, EntityHandle handle, double& x, double& y, double& z);

  static void centroid(Interface *MB, EntityHandle handle,Coord &coord);

  //static void edge_centers(Interface *MB, EntityHandle handle, std::vector<Coord> &coords_list);

  //static void face_centers(Interface *MB, EntityHandle handle, std::vector<Coord> &coords_list);

  template <typename T>
  inline static bool is_finite(T value)
  {
    return moab_isfinite(value);
  }

private:

  Util(){}

};




} // namespace moab

#endif
