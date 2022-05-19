//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_project_Registrar_h
#define smtk_extension_paraview_project_Registrar_h

#include "smtk/extension/paraview/project/smtkPQProjectExtModule.h"

#include "smtk/project/Manager.h"
#include "smtk/project/Registrar.h"

#include "smtk/operation/Manager.h"

#include "smtk/resource/Manager.h"

#include "smtk/view/Manager.h"

namespace smtk
{
namespace extension
{
namespace paraview
{
namespace project
{
class SMTKPQPROJECTEXT_EXPORT Registrar
{
public:
  using Dependencies = std::tuple<smtk::project::Registrar>;

  static void registerTo(const smtk::project::Manager::Ptr&);
  static void unregisterFrom(const smtk::project::Manager::Ptr&);

  static void registerTo(const smtk::view::Manager::Ptr&);
  static void unregisterFrom(const smtk::view::Manager::Ptr&);
};
} // namespace project
} // namespace paraview
} // namespace extension
} // namespace smtk
#endif
