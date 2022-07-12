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
#include "smtk/extension/paraview/appcomponents/Registrar.h"

#include "smtk/extension/paraview/appcomponents/GeometricVisibilityBadge.h"
#include "smtk/extension/paraview/appcomponents/HierarchicalVisibilityBadge.h"
#include "smtk/extension/paraview/appcomponents/VisibilityBadge.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResourceBrowser.h"

namespace smtk
{
namespace extension
{
namespace paraview
{
namespace appcomponents
{

namespace
{
using ViewWidgetList = std::tuple<pqSMTKResourceBrowser>;
using BadgeList =
  std::tuple<VisibilityBadge, GeometricVisibilityBadge, HierarchicalVisibilityBadge>;
} // namespace

void Registrar::registerTo(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->viewWidgetFactory().registerTypes<ViewWidgetList>();
  viewManager->badgeFactory().registerTypes<BadgeList>();
}

void Registrar::unregisterFrom(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->viewWidgetFactory().unregisterTypes<ViewWidgetList>();
  viewManager->badgeFactory().unregisterTypes<BadgeList>();
}
} // namespace appcomponents
} // namespace paraview
} // namespace extension
} // namespace smtk
