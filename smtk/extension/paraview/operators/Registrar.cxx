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
#ifndef PARAVIEW_VERSION_59
#include "smtk/extension/paraview/operators/smtkDataSetInfoInspectorView.h"
#include "smtk/extension/paraview/operators/smtkEditPropertiesView.h"
#include "smtk/extension/paraview/operators/smtkMeshInspectorView.h"
#endif

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
#ifdef PARAVIEW_VERSION_59
using ViewWidgetList = std::tuple<smtkAssignColorsView>;
#else
using ViewWidgetList = std::tuple<
  smtkAssignColorsView,
  smtkDataSetInfoInspectorView,
  smtkEditPropertiesView,
  smtkMeshInspectorView>;
#endif
} // namespace

void Registrar::registerTo(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->viewWidgetFactory().registerTypes<ViewWidgetList>();
}

void Registrar::unregisterFrom(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->viewWidgetFactory().unregisterTypes<ViewWidgetList>();
}
} // namespace operators
} // namespace paraview
} // namespace extension
} // namespace smtk
