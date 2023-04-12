//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/project/pqSMTKProjectMenu.h"

// Client side
#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqCoreUtilities.h"
#include "pqFileDialog.h"
#include "pqObjectBuilder.h"
#include "pqPropertiesPanel.h"
#include "pqServer.h"
#include "vtkSMPVRepresentationProxy.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxy.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/json/jsonResource.h"
#include "smtk/common/Color.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKRenderResourceBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"
#include "smtk/extension/qt/SVGIconEngine.h"
#include "smtk/extension/qt/qtOperationView.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/operation/Manager.h"
#include "smtk/project/operators/Create.h"
#include "smtk/project/view/IconConstructor.h"

#include <QAction>
#include <QApplication>
#include <QDialog>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QObject>
#include <QPushButton>
#include <QSharedPointer>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

pqNewProjectReaction::pqNewProjectReaction(QAction* parentObject)
  : Superclass(parentObject)
{
}

void pqNewProjectReaction::newProject()
{
  // Access the active server
  pqServer* server = pqActiveObjects::instance().activeServer();
  pqSMTKWrapper* wrapper = pqSMTKBehavior::instance()->resourceManagerForServer(server);

  // Access the unique name associated with the operation.
  auto createOp = wrapper->smtkOperationManager()->create<smtk::project::Create>();

  // Construct a modal dialog for the operation.
  QSharedPointer<QDialog> createDialog = QSharedPointer<QDialog>(new QDialog());
  createDialog->setObjectName("CreateProjectDialog");
  createDialog->setWindowTitle(
    QString::fromStdString(createOp->parameters()->definition()->label()));
  createDialog->setLayout(new QVBoxLayout(createDialog.data()));

  // Create a new UI for the dialog.
  QSharedPointer<smtk::extension::qtUIManager> uiManager =
    QSharedPointer<smtk::extension::qtUIManager>(new smtk::extension::qtUIManager(
      createOp, wrapper->smtkResourceManager(), wrapper->smtkViewManager()));

  // Create an operation view for the operation.
  smtk::view::ConfigurationPtr view = uiManager->findOrCreateOperationView();

  smtk::extension::qtOperationView* opView = dynamic_cast<smtk::extension::qtOperationView*>(
    uiManager->setSMTKView(view, createDialog.data()));

  // Remove all connections from the operation view's apply button. We are going
  // to intercept the client-side operation parameters and execute them on the
  // server.
  opView->applyButton()->disconnect();

  // Retrieve the operation parameters and close the modal dialog when the
  // apply button is clicked.
  std::string typeName;
  std::string parameters;
  QObject::connect(opView->applyButton(), &QPushButton::clicked, [&]() {
    typeName = opView->operation()->typeName();
    json j;
    smtk::attribute::to_json(j, opView->operation()->parameters());
    parameters = j.dump();
    createDialog->done(QDialog::Accepted);
  });

  // Launch the modal dialog and wait for the operation to succeed.
  createDialog->exec();

  if (!typeName.empty())
  {
    auto* pqCore = pqApplicationCore::instance();
    auto* builder = pqCore->getObjectBuilder();

    pqSMTKResource* src =
      static_cast<pqSMTKResource*>(builder->createSource("sources", "SMTKResourceCreator", server));
    vtkSMPropertyHelper(src->getProxy(), "TypeName").Set(typeName.c_str());
    vtkSMPropertyHelper(src->getProxy(), "Parameters").Set(parameters.c_str());

    pqSMTKRenderResourceBehavior::instance()->renderPipelineSource(src);
  }
}

namespace
{
QAction* findInsertionPoint(QMenu* menu)
{
  QListIterator<QAction*> actionIt(menu->actions());
  while (actionIt.hasNext())
  {
    if (actionIt.peekNext()->text().contains("new resource", Qt::CaseInsensitive))
    {
      return actionIt.next();
    }
    actionIt.next();
  }
  return nullptr;
}

QAction* findHelpMenuAction(QMenuBar* menubar)
{
  QList<QAction*> menuBarActions = menubar->actions();
  Q_FOREACH (QAction* existingMenuAction, menuBarActions)
  {
    QString menuName = existingMenuAction->text().toLower();
    menuName.remove('&');
    if (menuName == "help")
    {
      return existingMenuAction;
    }
  }
  return nullptr;
}
} // namespace

static pqSMTKProjectMenu* g_instance = nullptr;

pqSMTKProjectMenu::pqSMTKProjectMenu(QObject* parent)
  : Superclass(parent)
{
  auto* pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    QObject::connect(pqCore, &pqApplicationCore::clientEnvironmentDone, [this]() {
      QMainWindow* mainWindow = qobject_cast<QMainWindow*>(pqCoreUtilities::mainWidget());

      QList<QAction*> menuBarActions = mainWindow->menuBar()->actions();

      QMenu* menu = nullptr;
      Q_FOREACH (QAction* existingMenuAction, menuBarActions)
      {
        QString menuName = existingMenuAction->text();
        menuName.remove('&');
        if (menuName == "File")
        {
          menu = existingMenuAction->menu();
          break;
        }
      }

      std::string backgroundColor = "#ffffff";
      if (menu)
      {
        QColor bg = menu->palette().color(QPalette::Window);
        qreal bgF[3] = { bg.redF(), bg.greenF(), bg.blueF() };
        float lightness = smtk::common::Color::floatRGBToLightness(bgF);
        // white or black for our edge color, based on background.
        if (lightness >= 0.5)
        {
          backgroundColor = "#000000";
        }
      }

      QAction* newProjectAction = new QAction(
        QIcon(new smtk::extension::SVGIconEngine(
          smtk::project::view::IconConstructor()(backgroundColor))),
        tr("New &Project..."),
        this);
      newProjectAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_P));

      if (menu)
      {
        QAction* insertionPoint = findInsertionPoint(menu);
        menu->insertAction(insertionPoint, newProjectAction);
      }
      else
      {
        // If the File menu doesn't already exist, I don't think the following
        // logic works. It is taken from pqPluginActionGroupBehavior, which
        // is designed to accomplish pretty much the same task, though.

        // Create new menu.
        menu = new QMenu("File", mainWindow);
        menu->setObjectName("File");
        menu->addAction(newProjectAction);
        // insert new menus before the Help menu is possible.
        mainWindow->menuBar()->insertMenu(::findHelpMenuAction(mainWindow->menuBar()), menu);
      }
      new pqNewProjectReaction(newProjectAction);
    });
  }
}

pqSMTKProjectMenu* pqSMTKProjectMenu::instance(QObject* parent)
{
  if (!g_instance)
  {
    g_instance = new pqSMTKProjectMenu(parent);
  }

  if (g_instance->parent() == nullptr && parent)
  {
    g_instance->setParent(parent);
  }

  return g_instance;
}

pqSMTKProjectMenu::~pqSMTKProjectMenu()
{
  g_instance = nullptr;
  QObject::disconnect(this);
}
