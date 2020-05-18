//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_geometry_ClosestPoint_h
#define smtk_geometry_ClosestPoint_h

#include "smtk/CoreExports.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/query/DerivedFrom.h"
#include "smtk/resource/query/Query.h"

#include <array>

namespace smtk
{
namespace geometry
{

/**\brief An API for computing a the closest point on a geometric resource
  * component to an input point. The returned value represents a tessellation or
  * model vertex; no interpolation is performed.
  */
struct SMTKCORE_EXPORT ClosestPoint
  : public smtk::resource::query::DerivedFrom<ClosestPoint, smtk::resource::query::Query>
{
  virtual std::array<double, 3> operator()(
    const smtk::resource::Component::Ptr&,
    const std::array<double, 3>&) const = 0;
};

inline std::array<double, 3> ClosestPoint::operator()(
  const smtk::resource::Component::Ptr&,
  const std::array<double, 3>&) const
{
  static constexpr const double nan = std::numeric_limits<double>::quiet_NaN();
  return { { nan, nan, nan } };
}
} // namespace geometry
} // namespace smtk

#endif
