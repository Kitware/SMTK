//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_vtk_mesh_Registrar_h
#define smtk_extension_vtk_mesh_Registrar_h
#ifndef __VTK_WRAP__

#include "smtk/extension/vtk/mesh/vtkSMTKMeshExtModule.h"

#include "smtk/extension/vtk/source/Registrar.h"
#include "smtk/geometry/Manager.h"
#include "smtk/mesh/resource/Registrar.h"

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace mesh
{

class VTKSMTKMESHEXT_EXPORT Registrar
{
public:
  using Dependencies = std::tuple<mesh::Registrar, vtk::source::Registrar>;

  static void registerTo(const smtk::geometry::Manager::Ptr&);
  static void unregisterFrom(const smtk::geometry::Manager::Ptr&);
};
}
}
}
}

#endif // __VTK_WRAP__
#endif // smtk_extension_vtk_mesh_Registrar_h
