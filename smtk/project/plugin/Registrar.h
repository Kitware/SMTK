//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_project_plugin_Registrar_h
#define __smtk_project_plugin_Registrar_h

#include "smtk/project/Manager.h"
#include "smtk/project/Registrar.h"

#include "smtk/operation/Manager.h"

#include "smtk/resource/Manager.h"

#include "smtk/view/Manager.h"

namespace smtk
{
namespace project
{
namespace plugin
{
class Registrar
{
public:
  using Dependencies = std::tuple<smtk::project::Registrar>;

  static void registerTo(const smtk::project::Manager::Ptr&);
  static void unregisterFrom(const smtk::project::Manager::Ptr&);

  static void registerTo(const smtk::view::Manager::Ptr&);
  static void unregisterFrom(const smtk::view::Manager::Ptr&);
};
} // namespace plugin
} // namespace project
} // namespace smtk

#endif
