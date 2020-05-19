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
#include "smtk/extension/paraview/operators/Registrar.h"

#include "smtk/extension/paraview/operators/smtkAssignColorsView.h"

namespace smtk
{
namespace extension
{
namespace paraview
{
namespace operators
{

namespace
{
typedef std::tuple<smtkAssignColorsView> ViewWidgetList;
}

void Registrar::registerTo(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->viewWidgetFactory().registerTypes<ViewWidgetList>();
}

void Registrar::unregisterFrom(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->viewWidgetFactory().unregisterTypes<ViewWidgetList>();
}
}
}
}
}
