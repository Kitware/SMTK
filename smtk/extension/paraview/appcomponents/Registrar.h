//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_Registrar_h
#define smtk_extension_paraview_appcomponents_Registrar_h
#ifndef __VTK_WRAP__

#include "smtk/extension/paraview/appcomponents/smtkPQComponentsExtModule.h"
#include "smtk/extension/qt/qtViewRegistrar.h"

#include "smtk/common/Managers.h"
#include "smtk/resource/query/Manager.h"
#include "smtk/view/Manager.h"

namespace smtk
{
namespace extension
{
namespace paraview
{
namespace appcomponents
{

class SMTKPQCOMPONENTSEXT_EXPORT Registrar
{
public:
  using Dependencies = std::tuple<smtk::extension::qtViewRegistrar>;

  static void registerTo(const smtk::view::Manager::Ptr&);
  static void unregisterFrom(const smtk::view::Manager::Ptr&);
};
} // namespace appcomponents
} // namespace paraview
} // namespace extension
} // namespace smtk

#endif // __VTK_WRAP__
#endif // smtk_extension_paraview_appcomponents_Registrar_h
