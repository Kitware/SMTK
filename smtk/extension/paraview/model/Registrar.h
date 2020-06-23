//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_model_Registrar_h
#define smtk_extension_paraview_model_Registrar_h
#ifndef __VTK_WRAP__

#include "smtk/extension/paraview/model/smtkPVModelExtModule.h"

#include "smtk/extension/vtk/geometry/Registrar.h"
#include "smtk/model/Registrar.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Registrar.h"
#include "smtk/resource/Manager.h"

namespace smtk
{
namespace extension
{
namespace paraview
{
namespace model
{

class SMTKPVMODELEXT_EXPORT Registrar
{
public:
  using Dependencies = std::tuple<operation::Registrar, model::Registrar, vtk::geometry::Registrar>;

  static void registerTo(const smtk::operation::Manager::Ptr&);
  static void unregisterFrom(const smtk::operation::Manager::Ptr&);
};
}
}
}
}

#endif // __VTK_WRAP__
#endif // smtk_extension_paraview_model_Registrar_h
