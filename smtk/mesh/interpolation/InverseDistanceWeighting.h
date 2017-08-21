//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_mesh_InverseDistanceWeighting_h
#define __smtk_mesh_InverseDistanceWeighting_h

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
   via inverse distance weighting.

   Given an external data set of either structured or unstructured data, this
   functor is a continuous function from R^3->R whose values are computed as the
   inverse distance weights of the data set. Shepard's method is used to perform
   the computation.
  */
class SMTKCORE_EXPORT InverseDistanceWeighting
{
public:
  InverseDistanceWeighting(const PointCloud& pointcloud, double power = 1.);
  InverseDistanceWeighting(const StructuredGrid& structuredgrid, double power = 1.);

  double operator()(std::array<double, 3> x) const { return m_function(x); }

private:
  std::function<double(std::array<double, 3>)> m_function;
};
}
}

#endif
