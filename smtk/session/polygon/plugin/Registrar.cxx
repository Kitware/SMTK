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
#include "smtk/session/polygon/plugin/Registrar.h"

#ifdef VTK_SUPPORT
#include "smtk/session/polygon/plugin/qtExtractContoursView.h"
#endif
#include "smtk/session/polygon/plugin/qtPolygonEdgeOperationView.h"

namespace smtk
{
namespace session
{
namespace polygon
{
namespace plugin
{

namespace
{
typedef std::tuple<
#ifdef VTK_SUPPORT
  qtExtractContoursView,
#endif
  qtPolygonEdgeOperationView>
  ViewList;
}

void Registrar::registerTo(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->viewWidgetFactory().registerTypes<ViewList>();
#ifdef VTK_SUPPORT
  viewManager->viewWidgetFactory().addAlias<qtExtractContoursView>("smtkPolygonContourView");
#endif
  viewManager->viewWidgetFactory().addAlias<qtPolygonEdgeOperationView>("smtkPolygonEdgeView");
}

void Registrar::unregisterFrom(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->viewWidgetFactory().unregisterTypes<ViewList>();
}
}
}
}
}
