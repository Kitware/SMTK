//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_geometry_RandomPoint_h
#define smtk_geometry_RandomPoint_h

#include "smtk/CoreExports.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/query/DerivedFrom.h"
#include "smtk/resource/query/Query.h"

#include <array>

namespace smtk
{
namespace geometry
{

/**\brief An API for computing a random point on a geometric resource component.
  */
struct SMTKCORE_EXPORT RandomPoint
  : public smtk::resource::query::DerivedFrom<RandomPoint, smtk::resource::query::Query>
{
  virtual std::array<double, 3> operator()(const smtk::resource::Component::Ptr&) const = 0;

  virtual void seed(std::size_t) {}
};

inline std::array<double, 3> RandomPoint::operator()(const smtk::resource::Component::Ptr&) const
{
  static constexpr const double nan = std::numeric_limits<double>::quiet_NaN();
  return { { nan, nan, nan } };
}
} // namespace geometry
} // namespace smtk

#endif
