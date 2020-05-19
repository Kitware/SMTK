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

#include "smtk/extension/qt/MembershipBadge.h"
#include "smtk/extension/qt/TypeAndColorBadge.h"
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
using BadgeList =
  std::tuple<smtk::extension::qt::MembershipBadge, smtk::extension::qt::TypeAndColorBadge>;
}

void qtViewRegistrar::registerTo(const smtk::view::Manager::Ptr& manager)
{
  manager->viewWidgetFactory().registerTypes<ViewWidgetList>();
  // a set of user-friendly constructor names to use for alternate lookup.
  manager->viewWidgetFactory().addAlias<qtAnalysisView>("Analysis");
  manager->viewWidgetFactory().addAlias<qtAssociationView>("Associations");
  manager->viewWidgetFactory().addAlias<qtAttributeView>("Attribute");
  manager->viewWidgetFactory().addAlias<qtGroupView>("Group");
  manager->viewWidgetFactory().addAlias<qtInstancedView>("Instanced");
  manager->viewWidgetFactory().addAlias<qtOperationView>("Operation");
  manager->viewWidgetFactory().addAlias<qtSelectorView>("Selector");
  manager->viewWidgetFactory().addAlias<qtSimpleExpressionView>("SimpleExpression");
  manager->viewWidgetFactory().addAlias<qtCategorySelectorView>("Category");
  manager->viewWidgetFactory().addAlias<qtModelEntityAttributeView>("ModelEntity");
  manager->viewWidgetFactory().addAlias<qtResourceBrowser>("ResourceBrowser");

  manager->badgeFactory().registerTypes<BadgeList>();
}

void qtViewRegistrar::unregisterFrom(const smtk::view::Manager::Ptr& manager)
{
  manager->viewWidgetFactory().unregisterTypes<ViewWidgetList>();

  manager->badgeFactory().unregisterTypes<BadgeList>();
}
}
}
