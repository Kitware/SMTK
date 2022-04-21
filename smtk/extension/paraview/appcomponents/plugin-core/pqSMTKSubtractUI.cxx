//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/plugin-core/pqSMTKSubtractUI.h"

#include "smtk/io/Logger.h"

// Client side
#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqCoreUtilities.h"
#include "pqServerManagerModel.h"
#include "pqView.h"

#include <QAction>
#include <QApplication>
#include <QDockWidget>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>

// Change "#undef" to "#define" for debugging
#undef SMTK_DBG_SUBTRACT_UI

static pqSMTKSubtractUI* g_instance = nullptr;

static bool toggleMenuItem(QMenu* menu, bool hide)
{
  if (!menu)
  {
    return false;
  }
  // Both hide and disable the menu. This should help with
  // testing where we do not wish tests to succeed if users
  // could not have clicked on the menu to activate the action.
  menu->menuAction()->setVisible(!hide);
  menu->setEnabled(!hide);
  return true;
}

static QToolBar* findToolBar(QMainWindow* mainWindow, const QString& name)
{
  Q_FOREACH (QToolBar* bar, mainWindow->findChildren<QToolBar*>())
  {
    if (bar->windowTitle() == name)
    {
      return bar;
    }
  }
  return nullptr;
}

static bool hideToolBar(QMainWindow* mainWindow, const QString& name)
{
  auto* tb = findToolBar(mainWindow, name);
  if (tb)
  {
    tb->hide();
    return true;
  }
  return false;
}

static bool showToolBar(QMainWindow* mainWindow, const QString& name)
{
  auto* tb = findToolBar(mainWindow, name);
  if (tb)
  {
    tb->show();
    return true;
  }
  return false;
}

static QAction* findAction(QToolBar* toolbar, const QString& name)
{
  if (toolbar)
  {
    Q_FOREACH (QAction* act, toolbar->actions())
    {
      if (act->text() == name || act->objectName() == name)
      {
        return act;
      }
    }
  }
  return nullptr;
}

static QAction* findActionByName(QMainWindow* mainWindow, const QString& name)
{
  if (mainWindow)
  {
    Q_FOREACH (QAction* act, mainWindow->findChildren<QAction*>())
    {
      if (act->objectName() == name)
      {
        return act;
      }
    }
  }
  return nullptr;
}

static void toggleAction(QAction* act, bool hide)
{
  if (act)
  {
    act->setEnabled(!hide);
    act->setVisible(!hide);
  }
}

pqSMTKSubtractUI::pqSMTKSubtractUI(QObject* parent)
  : Superclass(parent)
{
}

pqSMTKSubtractUI* pqSMTKSubtractUI::instance(QObject* parent)
{
  if (!g_instance)
  {
    g_instance = new pqSMTKSubtractUI(parent);
  }

  if (g_instance->parent() == nullptr && parent)
  {
    g_instance->setParent(parent);
  }

  return g_instance;
}

pqSMTKSubtractUI::~pqSMTKSubtractUI()
{
  if (g_instance == this)
  {
    g_instance = nullptr;
  }

  QObject::disconnect(this);
}

void pqSMTKSubtractUI::toggleMenuItem(
  const std::string& itemPath,
  const std::string& itemSep,
  bool remove)
{
  // NB: Submenus are QMenu items but all menu items are QActions.
  // See https://stackoverflow.com/questions/9399840/how-to-iterate-through-a-menus-actions-in-qt
  // for more information.
  auto src = QString::fromStdString(itemPath);
  auto sep = QString::fromStdString(itemSep);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
  auto spl = src.split(sep, Qt::SkipEmptyParts);
#else
  auto spl = src.split(sep, QString::SkipEmptyParts);
#endif
  if (spl.empty())
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Empty menu item path.");
    return;
  }
  auto topLevelWidgets = QApplication::topLevelWidgets();
  for (QWidget* topLevelWidget : topLevelWidgets)
  {
    auto* mainWindow = dynamic_cast<QMainWindow*>(topLevelWidget);
    if (mainWindow)
    {
      QStringList hier = spl;
      QMenuBar* mb = mainWindow->menuBar();
      QMenu* mm = nullptr;
      auto menus = mb->findChildren<QMenu*>();
      for (auto* menu : menus)
      {
#ifdef SMTK_DBG_SUBTRACT_UI
        std::cout << "    Menu \"" << menu->objectName().toStdString() << "\""
                  << " title \"" << menu->title().toStdString() << "\"\n";
#endif
        if (menu->title() == hier.front())
        {
          mm = menu;
        }
      }
      if (!mm)
      {
        break;
      }
#ifdef SMTK_DBG_SUBTRACT_UI
      std::cout << "  Found " << mm->title().toStdString() << "\n";
#endif
      hier.pop_front();
      bool miss = false;
      while (!hier.empty())
      {
        auto menuActions = mm->actions();
        QMenu* menu;
        mm = nullptr;
        for (QAction* menuAction : menuActions)
        {
          if (menuAction->isSeparator())
          {
// Do nothing.
#ifdef SMTK_DBG_SUBTRACT_UI
            std::cout << "    ----\n";
#endif
          }
          else if ((menu = menuAction->menu()))
          {
#ifdef SMTK_DBG_SUBTRACT_UI
            std::cout << "    Menu \"" << menu->title().toStdString() << "\"\n";
#endif
            if (menu->title() == hier.front())
            {
              mm = menu;
            }
          }
          else
          {
#ifdef SMTK_DBG_SUBTRACT_UI
            std::cout << "    Action \"" << menuAction->objectName().toStdString() << "\""
                      << " (" << menuAction->text().toStdString() << ")\n";
#endif
            // Actions that are not menus must be leaf nodes.
            if (hier.size() == 1 && menuAction->text() == hier.front())
            {
              menuAction->setEnabled(!remove);
              menuAction->setVisible(!remove);
              return;
            }
          }
        }
        if (!mm)
        {
          miss = true;
          break;
        }
#ifdef SMTK_DBG_SUBTRACT_UI
        std::cout << "  Found  " << hier.front().toStdString() << "\n";
#endif
        hier.pop_front();
      }
      if (!miss)
      {
#ifdef SMTK_DBG_SUBTRACT_UI
        std::cout << "  Toggling  " << itemPath << "\n";
#endif
        if (::toggleMenuItem(mm, remove))
        {
          return;
        }
      }
    }
  }
  smtkErrorMacro(
    smtk::io::Logger::instance(),
    "  Could not toggle menu item \"" << itemPath << "\" " << (remove ? "off" : "on"));
}

void pqSMTKSubtractUI::toggleMenuItemByObjectName(const std::string& itemObjectName, bool remove)
{
  auto src = QString::fromStdString(itemObjectName);
  if (src.isEmpty())
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Empty menu item name.");
    return;
  }
  auto topLevelWidgets = QApplication::topLevelWidgets();
  for (QWidget* topLevelWidget : topLevelWidgets)
  {
    auto* mainWindow = dynamic_cast<QMainWindow*>(topLevelWidget);
    if (mainWindow)
    {
      auto* menu = mainWindow->findChild<QMenu*>(src);
      if (menu)
      {
        ::toggleMenuItem(menu, remove);
        return;
      }
      auto* action = mainWindow->findChild<QAction*>(src);
      if (action)
      {
        ::toggleAction(action, remove);
        return;
      }
    }
  }
  smtkErrorMacro(
    smtk::io::Logger::instance(),
    "  Could not toggle menu item \"" << itemObjectName << "\" " << (remove ? "off" : "on"));
}

void pqSMTKSubtractUI::toggleToolbar(const std::string& toolbar, bool remove)
{
  QString tbName = QString::fromStdString(toolbar);
  auto topLevelWidgets = QApplication::topLevelWidgets();
  for (QWidget* topLevelWidget : topLevelWidgets)
  {
    auto* mainWindow = dynamic_cast<QMainWindow*>(topLevelWidget);
    if (
      mainWindow &&
      ((remove && hideToolBar(mainWindow, tbName)) || (!remove && showToolBar(mainWindow, tbName))))
    {
      break;
    }
  }
}

void pqSMTKSubtractUI::toggleToolbarButton(
  const std::string& toolbar,
  const std::string& button,
  bool remove)
{
  QString tbName = QString::fromStdString(toolbar);
  auto topLevelWidgets = QApplication::topLevelWidgets();
  for (QWidget* topLevelWidget : topLevelWidgets)
  {
    auto* mainWindow = dynamic_cast<QMainWindow*>(topLevelWidget);
    auto* tb = ::findToolBar(mainWindow, tbName);
    if (tb)
    {
      QString buttonName = QString::fromStdString(button);
      auto* action = ::findAction(tb, buttonName);
      if (action)
      {
        action->setEnabled(!remove);
        action->setVisible(!remove);
        return;
      }
    }
  }
  smtkErrorMacro(
    smtk::io::Logger::instance(),
    "  Could not toggle toolbar \"" << toolbar
                                    << "\""
                                       " button \""
                                    << button << "\" " << (remove ? "off" : "on"));
}

void pqSMTKSubtractUI::toggleActionByObjectName(const std::string& action, bool remove)
{
  QString actionName = QString::fromStdString(action);
  auto topLevelWidgets = QApplication::topLevelWidgets();
  for (QWidget* topLevelWidget : topLevelWidgets)
  {
    auto* mainWindow = dynamic_cast<QMainWindow*>(topLevelWidget);
    auto* act = ::findActionByName(mainWindow, actionName);
    if (act)
    {
      act->setEnabled(!remove);
      act->setVisible(!remove);
    }
  }
  smtkErrorMacro(
    smtk::io::Logger::instance(),
    "  Could not toggle action \"" << action << "\" " << (remove ? "off" : "on"));
}

void pqSMTKSubtractUI::togglePanel(const std::string& panel, bool remove)
{
  QString panelName = QString::fromStdString(panel);
  auto topLevelWidgets = QApplication::topLevelWidgets();
  for (QWidget* topLevelWidget : topLevelWidgets)
  {
    auto* mainWindow = dynamic_cast<QMainWindow*>(topLevelWidget);
    if (mainWindow)
    {
      auto* dock = mainWindow->findChild<QDockWidget*>(panelName);
      if (!dock)
      {
        auto docks = mainWindow->findChildren<QDockWidget*>();
        for (auto* dd : docks)
        {
          if (dd->windowTitle() == panelName)
          {
            dock = dd;
            break;
          }
        }
      }
      if (dock)
      {
        auto* viewAction = dock->toggleViewAction();
        if (remove)
        {
#ifdef SMTK_DBG_SUBTRACT_UI
          std::cout << "closing " << dock->windowTitle().toStdString() << "\n";
#endif
          dock->close();
          viewAction->setEnabled(false);
          viewAction->setVisible(false);
          viewAction->setChecked(false);
        }
        else
        {
#ifdef SMTK_DBG_SUBTRACT_UI
          std::cout << "showing " << dock->windowTitle().toStdString() << "\n";
#endif
          dock->close();
          viewAction->setEnabled(true);
          viewAction->setVisible(true);
          viewAction->setChecked(true);
          dock->show();
        }
        return;
      }
    }
  }

  smtkErrorMacro(
    smtk::io::Logger::instance(),
    "  Could not toggle panel \"" << panel << "\" " << (remove ? "off" : "on"));
}
