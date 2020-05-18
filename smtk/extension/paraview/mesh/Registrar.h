//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_mesh_Registrar_h
#define smtk_extension_paraview_mesh_Registrar_h
#ifndef __VTK_WRAP__

#include "smtk/extension/paraview/mesh/smtkPVMeshExtModule.h"

#include "smtk/extension/vtk/mesh/Registrar.h"
#include "smtk/mesh/resource/Registrar.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Registrar.h"
#include "smtk/resource/Manager.h"
#include "smtk/view/Registrar.h"

namespace smtk
{
namespace extension
{
namespace paraview
{
namespace mesh
{

class SMTKPVMESHEXT_EXPORT Registrar
{
public:
  using Dependencies = std::tuple<operation::Registrar, mesh::Registrar, vtk::mesh::Registrar>;

  static void registerTo(const smtk::operation::Manager::Ptr&);
  static void unregisterFrom(const smtk::operation::Manager::Ptr&);
};
} // namespace mesh
} // namespace paraview
} // namespace extension
} // namespace smtk

#endif // __VTK_WRAP__
#endif // smtk_extension_paraview_mesh_Registrar_h
