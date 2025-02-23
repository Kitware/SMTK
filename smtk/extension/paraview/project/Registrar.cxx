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
#include "smtk/extension/paraview/project/Registrar.h"

#include "smtk/extension/paraview/project/pqSMTKProjectBrowser.h"
#include "smtk/extension/paraview/project/pqTaskControlView.h"

namespace smtk
{
namespace extension
{
namespace paraview
{
namespace project
{

using ViewWidgetList = std::tuple<pqSMTKProjectBrowser, pqTaskControlView>;

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
  (void)viewManager;
  viewManager->viewWidgetFactory().registerTypes<ViewWidgetList>();
  viewManager->viewWidgetFactory().addAlias<pqTaskControlView>("TaskControl");
}

void Registrar::unregisterFrom(const smtk::view::Manager::Ptr& viewManager)
{
  (void)viewManager;
  viewManager->viewWidgetFactory().unregisterTypes<ViewWidgetList>();
}
} // namespace project
} // namespace paraview
} // namespace extension
} // namespace smtk
