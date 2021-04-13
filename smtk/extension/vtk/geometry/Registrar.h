//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_vtk_geometry_Registrar_h
#define smtk_extension_vtk_geometry_Registrar_h
#ifndef __VTK_WRAP__

#include "smtk/extension/vtk/geometry/vtkSMTKGeometryExtModule.h"

#include "smtk/geometry/Manager.h"

#include "smtk/resource/query/Manager.h"

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace geometry
{

class VTKSMTKGEOMETRYEXT_EXPORT Registrar
{
public:
  static void registerTo(const smtk::geometry::Manager::Ptr&);
  static void unregisterFrom(const smtk::geometry::Manager::Ptr&);

  static void registerTo(const smtk::resource::query::Manager::Ptr&);
  static void unregisterFrom(const smtk::resource::query::Manager::Ptr&);
};
} // namespace geometry
} // namespace vtk
} // namespace extension
} // namespace smtk

#endif // __VTK_WRAP__
#endif // smtk_extension_vtk_geometry_Registrar_h
