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

class SMTKCORE_EXPORT InverseDistanceWeighting
{
public:
  InverseDistanceWeighting(const PointCloud& pointcloud, double power = 1.);
  InverseDistanceWeighting(const StructuredGrid& structuredgrid, double power = 1.);

#ifndef SHIBOKEN_SKIP
  double operator()(std::array<double, 3> x) const { return m_function(x); }
#endif

private:
  std::function<double(std::array<double, 3>)> m_function;
};
}
}

#endif
