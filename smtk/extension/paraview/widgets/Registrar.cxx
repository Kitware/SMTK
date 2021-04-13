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
#include "smtk/extension/paraview/widgets/Registrar.h"

#include "smtk/extension/paraview/widgets/qtSimpleExpressionEvaluationView.h"

namespace smtk
{
namespace extension
{
namespace paraview
{
namespace widgets
{

namespace
{
typedef std::tuple<qtSimpleExpressionEvaluationView> ViewWidgetList;
}

void Registrar::registerTo(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->viewWidgetFactory().registerTypes<ViewWidgetList>();
  // Note this should override the default for SimpleExpression
  viewManager->viewWidgetFactory().addAlias<qtSimpleExpressionEvaluationView>("SimpleExpression");
}

void Registrar::unregisterFrom(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->viewWidgetFactory().unregisterTypes<ViewWidgetList>();
}
} // namespace widgets
} // namespace paraview
} // namespace extension
} // namespace smtk
