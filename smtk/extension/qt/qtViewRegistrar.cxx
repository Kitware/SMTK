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
#include "smtk/extension/qt/qtViewRegistrar.h"

#include "smtk/extension/qt/qtAnalysisView.h"
#include "smtk/extension/qt/qtAssociationView.h"
#include "smtk/extension/qt/qtAttributeView.h"
#include "smtk/extension/qt/qtCategorySelectorView.h"
#include "smtk/extension/qt/qtGroupView.h"
#include "smtk/extension/qt/qtInstancedView.h"
#include "smtk/extension/qt/qtModelEntityAttributeView.h"
#include "smtk/extension/qt/qtOperationView.h"
#include "smtk/extension/qt/qtResourceBrowser.h"
#include "smtk/extension/qt/qtSelectorView.h"
#include "smtk/extension/qt/qtSimpleExpressionView.h"

#include <tuple>

namespace smtk
{
namespace extension
{
namespace
{
typedef std::tuple<qtAnalysisView, qtAssociationView, qtAttributeView, qtCategorySelectorView,
  qtGroupView, qtInstancedView, qtModelEntityAttributeView, qtOperationView, qtResourceBrowser,
  qtSelectorView, qtSimpleExpressionView>
  ViewWidgetList;
}

void qtViewRegistrar::registerTo(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->registerViewWidgets<ViewWidgetList>();
  // a set of user-friendly constructor names to use for alternate lookup.
  viewManager->addWidgetAlias<qtAnalysisView>("Analysis");
  viewManager->addWidgetAlias<qtAssociationView>("Associations");
  viewManager->addWidgetAlias<qtAttributeView>("Attribute");
  viewManager->addWidgetAlias<qtGroupView>("Group");
  viewManager->addWidgetAlias<qtInstancedView>("Instanced");
  viewManager->addWidgetAlias<qtOperationView>("Operation");
  viewManager->addWidgetAlias<qtSelectorView>("Selector");
  viewManager->addWidgetAlias<qtSimpleExpressionView>("SimpleExpression");
  viewManager->addWidgetAlias<qtCategorySelectorView>("Category");
  viewManager->addWidgetAlias<qtModelEntityAttributeView>("ModelEntity");
  viewManager->addWidgetAlias<qtResourceBrowser>("ResourceBrowser");
}

void qtViewRegistrar::unregisterFrom(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->unregisterViewWidgets<ViewWidgetList>();
}
}
}
