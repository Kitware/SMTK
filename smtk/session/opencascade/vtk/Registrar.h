//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_session_opencascade_vtk_Registrar_h
#define smtk_session_opencascade_vtk_Registrar_h

#include "smtk/session/opencascade/vtk/vtkOpencascadeGeometryExtModule.h"

#include "smtk/resource/Manager.h"
#include "smtk/session/opencascade/Registrar.h"

namespace smtk
{
namespace session
{
namespace opencascade
{
namespace vtk
{

class VTKOPENCASCADEGEOMETRYEXT_EXPORT Registrar
{
public:
  using Dependencies = std::tuple<smtk::session::opencascade::Registrar>;

  static void registerTo(const smtk::resource::Manager::Ptr&);
  static void unregisterFrom(const smtk::resource::Manager::Ptr&);
};
}
}
}
}

#endif
