//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_geometry_DistanceTo_h
#define smtk_geometry_DistanceTo_h

#include "smtk/CoreExports.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/query/DerivedFrom.h"
#include "smtk/resource/query/Query.h"

#include <array>
#include <utility>

namespace smtk
{
namespace geometry
{

/**\brief An API for computing the shortest distance between an input point and
  * a geometric resource component. The location of the point on the component
  * is also returned. This query differs from ClosestPoint in that the returned
  * point does not need to be explicitly contained within the geometric
  * representation.
  */
struct SMTKCORE_EXPORT DistanceTo
  : public smtk::resource::query::DerivedFrom<DistanceTo, smtk::resource::query::Query>
{
  virtual std::pair<double, std::array<double, 3> > operator()(
    const smtk::resource::Component::Ptr&, const std::array<double, 3>&) const = 0;
};

inline std::pair<double, std::array<double, 3> > DistanceTo::operator()(
  const smtk::resource::Component::Ptr&, const std::array<double, 3>&) const
{
  static constexpr const double nan = std::numeric_limits<double>::quiet_NaN();
  return std::make_pair(nan, std::array<double, 3>({ nan, nan, nan }));
}
}
}

#endif
