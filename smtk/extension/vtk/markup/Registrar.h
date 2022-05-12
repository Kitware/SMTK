//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_vtk_markup_Registrar_h
#define smtk_extension_vtk_markup_Registrar_h
#ifndef __VTK_WRAP__

#include "smtk/extension/vtk/markup/vtkSMTKMarkupExtModule.h"

#include "smtk/extension/vtk/geometry/Registrar.h"
#include "smtk/geometry/Manager.h"
#include "smtk/markup/Registrar.h"

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace markup
{

class VTKSMTKMARKUPEXT_EXPORT Registrar
{
public:
  using Dependencies = std::tuple<markup::Registrar, vtk::geometry::Registrar>;

  static void registerTo(const smtk::geometry::Manager::Ptr&);
  static void unregisterFrom(const smtk::geometry::Manager::Ptr&);
};
} // namespace markup
} // namespace vtk
} // namespace extension
} // namespace smtk

#endif // __VTK_WRAP__
#endif // smtk_extension_vtk_markup_Registrar_h
