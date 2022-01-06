//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_vtk_operators_Registrar_h
#define smtk_extension_vtk_operators_Registrar_h

#include "smtk/extension/vtk/operators/vtkSMTKOperationsExtModule.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Registrar.h"
#include "smtk/resource/Registrar.h"
#include "smtk/view/Manager.h"

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace operators
{
class VTKSMTKOPERATIONSEXT_EXPORT Registrar
{
public:
  using Dependencies = std::tuple<resource::Registrar, operation::Registrar>;

  static void registerTo(const smtk::operation::Manager::Ptr&);
  static void unregisterFrom(const smtk::operation::Manager::Ptr&);

  static void registerTo(const smtk::view::Manager::Ptr&);
  static void unregisterFrom(const smtk::view::Manager::Ptr&);
};
} // namespace operators
} // namespace vtk
} // namespace extension
} // namespace smtk

#endif
