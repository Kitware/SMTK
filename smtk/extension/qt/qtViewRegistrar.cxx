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
#include "smtk/extension/qt/diagram/qtComponentNode.h"
#include "smtk/extension/qt/diagram/qtConnectMode.h"
#include "smtk/extension/qt/diagram/qtDefaultTaskNode.h"
#include "smtk/extension/qt/diagram/qtDefaultTaskNode1.h"
#include "smtk/extension/qt/diagram/qtDiagram.h"
#include "smtk/extension/qt/diagram/qtDisconnectMode.h"
#include "smtk/extension/qt/diagram/qtPanMode.h"
#include "smtk/extension/qt/diagram/qtResourceDiagram.h"
#include "smtk/extension/qt/diagram/qtResourceNode.h"
#include "smtk/extension/qt/diagram/qtSelectMode.h"
#include "smtk/extension/qt/diagram/qtTaskEditor.h"
#include "smtk/extension/qt/diagram/qtTaskNode.h"
#include "smtk/extension/qt/qtAnalysisView.h"
#include "smtk/extension/qt/qtAssociationView.h"
#include "smtk/extension/qt/qtAttributeView.h"
#include "smtk/extension/qt/qtCategorySelectorView.h"
#include "smtk/extension/qt/qtComponentAttributeView.h"
#include "smtk/extension/qt/qtGroupView.h"
#include "smtk/extension/qt/qtInstancedView.h"
#include "smtk/extension/qt/qtOperationPalette.h"
#include "smtk/extension/qt/qtOperationView.h"
#include "smtk/extension/qt/qtResourceBrowser.h"
#include "smtk/extension/qt/qtSelectorView.h"
#include "smtk/extension/qt/qtSimpleExpressionView.h"
#include "smtk/extension/qt/qtWorkletPalette.h"

#include "smtk/plugin/Manager.h"

#include "smtk/Options.h"

#include <tuple>

#if SMTK_ENABLE_PYTHON_WRAPPING
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Resource.h"
#include "smtk/operation/pybind11/PyOperation.h"
#endif

#include <QApplication>
#include <QCoreApplication>
#include <QFontDatabase>
#include <QTimer>
#include <QtDebug>

namespace smtk
{
namespace extension
{
namespace
{
using ViewWidgetList = std::tuple<
  qtAnalysisView,
  qtAssociationView,
  qtAttributeView,
  qtCategorySelectorView,
  qtGroupView,
  qtInstancedView,
  qtComponentAttributeView,
  qtOperationView,
  qtOperationPalette,
  qtResourceBrowser,
  qtSelectorView,
  qtSimpleExpressionView,
  qtDiagram,
  qtWorkletPalette>;

using BadgeList =
  std::tuple<smtk::extension::qt::MembershipBadge, smtk::extension::qt::TypeAndColorBadge>;

using DiagramGeneratorList = std::tuple<qtTaskEditor, qtResourceDiagram>;
using DiagramViewModeList = std::tuple<qtConnectMode, qtDisconnectMode, qtPanMode, qtSelectMode>;
using TaskNodeList = std::tuple<qtTaskNode, qtDefaultTaskNode, qtDefaultTaskNode1>;
using ObjectNodeList = std::tuple<qtResourceNode, qtComponentNode>;

} // namespace

void qtViewRegistrar::registerTo(const smtk::common::Managers::Ptr& managers)
{
  managers->insert(qtManager::create());
  smtk::plugin::Manager::instance()->registerPluginsTo(managers->get<qtManager::Ptr>());

#if SMTK_ENABLE_PYTHON_WRAPPING
  smtk::operation::PyOperation::runOnMainThread =
    [](smtk::operation::PyOperation::SimpleFunction fn) {
      if (!QCoreApplication::instance() || QThread::currentThread() == qApp->thread())
      {
        // We're running in the GUI thread already, just call the function:
        fn();
        return;
      }

      QMetaObject::invokeMethod(qApp, fn, Qt::BlockingQueuedConnection);
    };
#endif
}

void qtViewRegistrar::unregisterFrom(const smtk::common::Managers::Ptr& managers)
{
  managers->erase<qtManager>();

#if SMTK_ENABLE_PYTHON_WRAPPING
  smtk::operation::PyOperation::runOnMainThread =
    [](smtk::operation::PyOperation::SimpleFunction fn) { fn(); };
#endif
}

void qtViewRegistrar::registerTo(const smtk::extension::qtManager::Ptr& qtMgr)
{
  qtMgr->diagramViewModeFactory().registerTypes<DiagramViewModeList>();
  qtMgr->diagramGeneratorFactory().registerTypes<DiagramGeneratorList>();
  qtMgr->taskNodeFactory().registerTypes<TaskNodeList>();
  qtMgr->objectNodeFactory().registerTypes<ObjectNodeList>();

  // If there is a Qt Application initialized then we need to add some additional
  // fonts that are used by classes such as qtTaskNode
  if (QCoreApplication::instance())
  {
    if (QFontDatabase::addApplicationFont(":/fonts/fontAwesomeRegular.otf") < 0)
    {
      qWarning() << "FontAwesomeRegular cannot be loaded !";
    }
    if (QFontDatabase::addApplicationFont(":/fonts/fontAwesomeSolid.otf") < 0)
    {
      qWarning() << "FontAwesomeSolid cannot be loaded !";
    }
  }
}

void qtViewRegistrar::unregisterFrom(const smtk::extension::qtManager::Ptr& qtMgr)
{
  qtMgr->diagramViewModeFactory().unregisterTypes<DiagramViewModeList>();
  qtMgr->diagramGeneratorFactory().unregisterTypes<DiagramGeneratorList>();
  qtMgr->taskNodeFactory().unregisterTypes<TaskNodeList>();
  qtMgr->objectNodeFactory().unregisterTypes<ObjectNodeList>();
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
  manager->viewWidgetFactory().addAlias<qtOperationPalette>("OperationPalette");
  manager->viewWidgetFactory().addAlias<qtOperationView>("Operation");
  manager->viewWidgetFactory().addAlias<qtSelectorView>("Selector");
  manager->viewWidgetFactory().addAlias<qtSimpleExpressionView>("SimpleExpression");
  manager->viewWidgetFactory().addAlias<qtCategorySelectorView>("Category");
  // Keeping this for backward compatibility for the time being
  manager->viewWidgetFactory().addAlias<qtComponentAttributeView>("ModelEntity");
  manager->viewWidgetFactory().addAlias<qtComponentAttributeView>("ComponentAttribute");
  manager->viewWidgetFactory().addAlias<qtResourceBrowser>("ResourceBrowser");
  manager->viewWidgetFactory().addAlias<qtDiagram>("Diagram");
  manager->viewWidgetFactory().addAlias<qtWorkletPalette>("WorkletPalette");

  manager->badgeFactory().registerTypes<BadgeList>();
}

void qtViewRegistrar::unregisterFrom(const smtk::view::Manager::Ptr& manager)
{
  manager->viewWidgetFactory().unregisterTypes<ViewWidgetList>();

  manager->badgeFactory().unregisterTypes<BadgeList>();
}
} // namespace extension
} // namespace smtk
