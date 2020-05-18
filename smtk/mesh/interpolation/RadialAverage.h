//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_RadialAverage_h
#define __smtk_mesh_RadialAverage_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"

#include <array>
#include <functional>

namespace smtk
{
namespace mesh
{

class PointCloud;
class StructuredGrid;

/**\brief A functor that converts an external data set into a continuous field
   via radial averaging.

   Given an external data set of either structured or unstructured data, this
   functor is a continuous function from R^3->R whose values are computed as the
   average of the points in the data set within a cylinder of radius \a radius
   axis-aligned with the z axis and centered at the input point. Values from the
   input data set can be masked using the prefilter functor.
  */
class SMTKCORE_EXPORT RadialAverage
{
public:
  RadialAverage(
    ResourcePtr collection,
    const PointCloud&,
    double radius,
    std::function<bool(double)> prefilter = [](double) { return true; });
  RadialAverage(
    const StructuredGrid&,
    double radius,
    std::function<bool(double)> prefilter = [](double) { return true; });

  double operator()(std::array<double, 3> x) const { return m_function(x); }

private:
  std::function<double(std::array<double, 3>)> m_function;
};
} // namespace mesh
} // namespace smtk

#endif
