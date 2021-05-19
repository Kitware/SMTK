//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_vtk_geometry_Backend_h
#define smtk_vtk_geometry_Backend_h

#include "smtk/extension/vtk/geometry/vtkSMTKGeometryExtModule.h"
#include "smtk/geometry/Backend.h"
#include "smtk/geometry/GeometryForBackend.h"

#include "vtkSmartPointer.h"

#include <memory>
#include <string>

class vtkDataObject;

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace geometry
{

/**\brief This class is used to indicate VTK exists as a consumer of renderable geometry.
  *
  * You may optionally construct the backend with a geometry provider and
  * avoid the cost of dynamic casting each time geometry is requested for an object.
  */
class VTKSMTKGEOMETRYEXT_EXPORT Backend : public smtk::geometry::Backend
{
public:
  using format_t = vtkSmartPointer<vtkDataObject>;

  std::string name() const override { return "VTK"; }
};

} // namespace geometry
} // namespace vtk
} // namespace extension
} // namespace smtk

#endif // smtk_vtk_geometry_Backend_h
