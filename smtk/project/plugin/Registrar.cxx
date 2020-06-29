//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#include "smtk/project/plugin/Registrar.h"

#include "smtk/project/plugin/pqSMTKProjectBrowser.h"

namespace smtk
{
namespace project
{
namespace plugin
{
void Registrar::registerTo(const smtk::project::Manager::Ptr& projectManager)
{
  projectManager->registerProject("basic");
}

void Registrar::unregisterFrom(const smtk::project::Manager::Ptr& projectManager)
{
  projectManager->unregisterProject("basic");
}

void Registrar::registerTo(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->viewWidgetFactory().registerType<pqSMTKProjectBrowser>();
}

void Registrar::unregisterFrom(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->viewWidgetFactory().unregisterType<pqSMTKProjectBrowser>();
}
} // namespace plugin
} // namespace project
} // namespace smtk
