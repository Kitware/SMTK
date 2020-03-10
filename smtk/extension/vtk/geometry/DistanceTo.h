//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_vtk_geometry_DistanceTo_h
#define smtk_extension_vtk_geometry_DistanceTo_h

#include "smtk/extension/vtk/geometry/vtkSMTKGeometryExtModule.h"

#include "smtk/geometry/queries/DistanceTo.h"

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace geometry
{

/**\brief An API for computing the shortest distance between an input point and
  * a geometric resource component. The location of the point on the component
  * is also returned. This query differs from ClosestPoint in that the returned
  * point does not need to be explicitly contained within the geometric
  * representation.
  */
struct VTKSMTKGEOMETRYEXT_EXPORT DistanceTo
  : public smtk::resource::query::DerivedFrom<DistanceTo, smtk::geometry::DistanceTo>
{
  std::pair<double, std::array<double, 3> > operator()(
    const smtk::resource::Component::Ptr&, const std::array<double, 3>&) const override;
};
}
}
}
}

#endif
