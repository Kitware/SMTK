//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_vtk_geometry_BoundingBox_h
#define smtk_extension_vtk_geometry_BoundingBox_h

#include "smtk/extension/vtk/geometry/vtkSMTKGeometryExtModule.h"

#include "smtk/geometry/queries/BoundingBox.h"

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace geometry
{
/**\brief Use the VTK description of the object to compute its bounding box..
  */
struct VTKSMTKGEOMETRYEXT_EXPORT BoundingBox
  : public smtk::resource::query::DerivedFrom<BoundingBox, smtk::geometry::BoundingBox>
{
  std::array<double, 6> operator()(const smtk::resource::PersistentObject::Ptr&) const override;
};
}
}
}
}

#endif
