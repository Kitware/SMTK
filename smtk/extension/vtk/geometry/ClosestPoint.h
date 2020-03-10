//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_vtk_geometry_ClosestPoint_h
#define smtk_extension_vtk_geometry_ClosestPoint_h

#include "smtk/extension/vtk/geometry/vtkSMTKGeometryExtModule.h"

#include "smtk/geometry/queries/ClosestPoint.h"

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace geometry
{

/**\brief An API for computing a the closest point on a geometric resource
  * component to an input point. The returned value represents a tessellation or
  * model vertex; no interpolation is performed.
  */
struct VTKSMTKGEOMETRYEXT_EXPORT ClosestPoint
  : public smtk::resource::query::DerivedFrom<ClosestPoint, smtk::geometry::ClosestPoint>
{
  std::array<double, 3> operator()(
    const smtk::resource::Component::Ptr&, const std::array<double, 3>&) const override;
};
}
}
}
}

#endif
