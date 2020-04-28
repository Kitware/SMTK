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
typedef std::tuple<pqSMTKResourceBrowser> ViewWidgetList;
}

void Registrar::registerTo(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->registerViewWidgets<ViewWidgetList>();
  viewManager->badgeFactory().registerBadge<VisibilityBadge>();
}

void Registrar::unregisterFrom(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->unregisterViewWidgets<ViewWidgetList>();
  viewManager->badgeFactory().unregisterBadge<VisibilityBadge>();
}
}
}
}
}
