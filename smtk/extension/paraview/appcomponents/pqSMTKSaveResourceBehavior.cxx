//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKSaveResourceBehavior.h"

// Client side
#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqCoreUtilities.h"
#include "pqFileDialog.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqServer.h"
#include "vtkSMProxy.h"
#include "vtkSMProxyManager.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/common/Paths.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/operators/WriteResource.h"
#include "smtk/resource/Manager.h"

#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QObject>
#include <QTextStream>

void initSaveResourceBehaviorResources()
{
  Q_INIT_RESOURCE(pqSMTKSaveResourceBehavior);
}

//-----------------------------------------------------------------------------
pqSaveResourceReaction::pqSaveResourceReaction(QAction* parentObject)
  : Superclass(parentObject)
{
  pqActiveObjects* activeObjects = &pqActiveObjects::instance();
  QObject::connect(
    activeObjects, SIGNAL(serverChanged(pqServer*)), this, SLOT(updateEnableState()));
  QObject::connect(
    activeObjects, SIGNAL(sourceChanged(pqPipelineSource*)), this, SLOT(updateEnableState()));
  this->updateEnableState();
}

//-----------------------------------------------------------------------------
void pqSaveResourceReaction::updateEnableState()
{
  pqActiveObjects& activeObjects = pqActiveObjects::instance();
  // TODO: also if there's a pending accept.
  bool enable_state =
    (activeObjects.activeServer() != nullptr && activeObjects.activeSource() != nullptr &&
     dynamic_cast<pqSMTKResource*>(activeObjects.activeSource()) != nullptr);
  this->parentAction()->setEnabled(enable_state);
}

//-----------------------------------------------------------------------------
pqSaveResourceReaction::State pqSaveResourceReaction::saveResource(pqSMTKResource* smtkResource)
{
  pqActiveObjects& activeObjects = pqActiveObjects::instance();
  pqSMTKResource* activeResource = smtkResource != nullptr
    ? smtkResource
    : dynamic_cast<pqSMTKResource*>(activeObjects.activeSource());
  if (!activeResource)
  {
    return pqSaveResourceReaction::State::Failed;
  }
  smtk::resource::ResourcePtr resource = activeResource->getResource();

  // If there is no associated file name with this resource, call
  // pqSaveResourceAsReaction's saveResourceAs static method instead.
  if (resource->location().empty())
  {
    return pqSaveResourceAsReaction::saveResourceAs(activeResource);
  }

  // Ensure the resource suffix is .smtk so readers can subsequently read the file.
  std::string filename = resource->location();
  std::string ext = smtk::common::Paths::extension(filename);
  if (ext != ".smtk")
  {
    qWarning() << "Invalid resource filename " << QString::fromStdString(filename)
               << ": must have  \".smtk\" extension";
    return pqSaveResourceReaction::State::Aborted;
  }

  auto* wrapper = pqSMTKBehavior::instance()->resourceManagerForServer(activeResource->getServer());
  if (smtk::resource::Manager::Ptr manager = resource->manager())
  {
    std::shared_ptr<smtk::common::Managers> managers =
      wrapper ? wrapper->smtkManagersPtr() : nullptr;
    // The resource manager returns true on success
    return manager->write(resource, managers) ? pqSaveResourceReaction::State::Succeeded
                                              : pqSaveResourceReaction::State::Failed;
  }
  else
  {
    // the standard way of saving doesn't work, because the deleted
    // pipeline object doesn't have a resource manager.
    // Instead, directly call WriteResource:
    pqServer* server = activeObjects.activeServer();
    pqSMTKWrapper* wrapper = pqSMTKBehavior::instance()->resourceManagerForServer(server);
    if (!server || !wrapper)
    {
      return pqSaveResourceReaction::State::Failed;
    }
    smtk::operation::WriteResource::Ptr writeOp =
      wrapper->smtkOperationManager()->create<smtk::operation::WriteResource>();

    writeOp->parameters()->associate(resource);
    writeOp->parameters()->findFile("filename")->setIsEnabled(true);
    writeOp->parameters()->findFile("filename")->setValue(filename);

    smtk::operation::Operation::Result writeOpResult = writeOp->operate();
    return writeOpResult->findInt("outcome")->value() ==
        static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED)
      ? pqSaveResourceReaction::State::Succeeded
      : pqSaveResourceReaction::State::Failed;
  }

  return pqSaveResourceReaction::State::Failed;
}

//-----------------------------------------------------------------------------
pqSaveResourceAsReaction::pqSaveResourceAsReaction(QAction* parentObject)
  : Superclass(parentObject)
{
  pqActiveObjects* activeObjects = &pqActiveObjects::instance();
  QObject::connect(
    activeObjects, SIGNAL(serverChanged(pqServer*)), this, SLOT(updateEnableState()));
  QObject::connect(
    activeObjects, SIGNAL(sourceChanged(pqPipelineSource*)), this, SLOT(updateEnableState()));
  this->updateEnableState();
}

//-----------------------------------------------------------------------------
void pqSaveResourceAsReaction::updateEnableState()
{
  pqActiveObjects& activeObjects = pqActiveObjects::instance();
  // TODO: also is there's a pending accept.
  bool enable_state =
    (activeObjects.activeServer() != nullptr && activeObjects.activeSource() != nullptr &&
     dynamic_cast<pqSMTKResource*>(activeObjects.activeSource()) != nullptr);
  this->parentAction()->setEnabled(enable_state);
}

//-----------------------------------------------------------------------------
pqSaveResourceReaction::State pqSaveResourceAsReaction::saveResourceAs(pqSMTKResource* smtkResource)
{
  pqServer* server = pqActiveObjects::instance().activeServer();
  pqActiveObjects& activeObjects = pqActiveObjects::instance();
  pqSMTKResource* activeResource = smtkResource != nullptr
    ? smtkResource
    : dynamic_cast<pqSMTKResource*>(activeObjects.activeSource());
  if (!activeResource)
  {
    return pqSaveResourceReaction::State::Failed;
  }
  smtk::resource::ResourcePtr resource = activeResource->getResource();

  QString filters("Simulation Modeling Toolkit Resource files (*.smtk)");
  QString title(tr("Save File: "));
  // add a name so user can tell which resource is being saved.
  title += resource->name().c_str();

  pqFileDialog fileDialog(
    server,
    pqCoreUtilities::mainWidget(),
    title,
    (resource->location().empty()
       ? QString()
       : QFileInfo(QString::fromStdString(resource->location())).absoluteDir().absolutePath()),
    filters);
  fileDialog.setObjectName("FileSaveDialog");
  fileDialog.setFileMode(pqFileDialog::AnyFile);

  if (fileDialog.exec() == QDialog::Accepted)
  {
    QString qname = fileDialog.getSelectedFiles()[0];
    std::string filename = qname.toStdString();

    // Ensure the resource suffix is .smtk so readers can subsequently read the file.
    std::string ext = smtk::common::Paths::extension(filename);
    if (ext.empty())
    {
      // If no extension provided, then add .smtk
      filename += ".smtk";
    }
    else if (ext != ".smtk")
    {
      // But if extension is not .smtk, then abort
      QWidget* mainWidget = pqCoreUtilities::mainWidget();
      QString text;
      QTextStream qs(&text);
      qs << "Invalid filename " << qname << ": must have  \".smtk\" extension";
      QMessageBox::critical(mainWidget, "Invalid File Name", text);
      return pqSaveResourceReaction::State::Aborted;
    }

    auto* wrapper =
      pqSMTKBehavior::instance()->resourceManagerForServer(activeResource->getServer());
    if (smtk::resource::Manager::Ptr manager = resource->manager())
    {
      std::shared_ptr<smtk::common::Managers> managers =
        wrapper ? wrapper->smtkManagersPtr() : nullptr;
      return manager->write(resource, filename, managers) ? pqSaveResourceReaction::State::Succeeded
                                                          : pqSaveResourceReaction::State::Failed;
    }
    else
    {
      // the standard way of saving doesn't work, because the deleted
      // pipeline object doesn't have a resource manager.
      // Instead, directly call WriteResource:
      pqServer* server = activeObjects.activeServer();
      pqSMTKWrapper* wrapper = pqSMTKBehavior::instance()->resourceManagerForServer(server);
      if (!server || !wrapper)
      {
        return pqSaveResourceReaction::State::Failed;
      }
      smtk::operation::WriteResource::Ptr writeOp =
        wrapper->smtkOperationManager()->create<smtk::operation::WriteResource>();

      writeOp->parameters()->associate(resource);
      writeOp->parameters()->findFile("filename")->setIsEnabled(true);
      writeOp->parameters()->findFile("filename")->setValue(filename);

      smtk::operation::Operation::Result writeOpResult = writeOp->operate();
      return writeOpResult->findInt("outcome")->value() ==
          static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED)
        ? pqSaveResourceReaction::State::Succeeded
        : pqSaveResourceReaction::State::Failed;
    }
  }
  return pqSaveResourceReaction::State::Aborted;
}

namespace
{
QAction* findSaveAction(QMenu* menu)
{
  Q_FOREACH (QAction* action, menu->actions())
  {
    if (action->text().contains("save data", Qt::CaseInsensitive))
    {
      return action;
    }
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

static pqSMTKSaveResourceBehavior* g_instance = nullptr;

pqSMTKSaveResourceBehavior::pqSMTKSaveResourceBehavior(QObject* parent)
  : Superclass(parent)
{
  initSaveResourceBehaviorResources();

  auto* pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    QObject::connect(pqCore, &pqApplicationCore::clientEnvironmentDone, [this]() {
      QAction* saveResourceAction =
        new QAction(QPixmap(":/SaveResourceBehavior/Save24.png"), tr("&Save Resource"), this);
      saveResourceAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
      QAction* saveResourceAsAction =
        new QAction(QPixmap(":/SaveResourceBehavior/Save24.png"), tr("&Save Resource As..."), this);
      saveResourceAsAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_S));

      QMainWindow* mainWindow = qobject_cast<QMainWindow*>(pqCoreUtilities::mainWidget());

      // Stop here if main window does not exist
      // This is typically the case for certain unit tests.
      if (!mainWindow)
      {
        return;
      }

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

      if (menu)
      {
        QAction* saveAction = findSaveAction(menu);

        menu->insertAction(saveAction, saveResourceAsAction);
        menu->insertAction(saveResourceAsAction, saveResourceAction);
      }
      else
      {
        // If the File menu doesn't already exist, I don't think the following
        // logic works. It is taken from pqPluginActionGroupBehavior, which
        // is designed to accomplish pretty much the same task, though.

        // Create new menu.
        menu = new QMenu("File", mainWindow);
        menu->setObjectName("File");
        menu->addAction(saveResourceAsAction);
        menu->addAction(saveResourceAction);
        // insert new menus before the Help menu is possible.
        mainWindow->menuBar()->insertMenu(::findHelpMenuAction(mainWindow->menuBar()), menu);
      }
      new pqSaveResourceReaction(saveResourceAction);
      new pqSaveResourceAsReaction(saveResourceAsAction);
    });
  }
}

pqSMTKSaveResourceBehavior* pqSMTKSaveResourceBehavior::instance(QObject* parent)
{
  if (!g_instance)
  {
    g_instance = new pqSMTKSaveResourceBehavior(parent);
  }

  if (g_instance->parent() == nullptr && parent)
  {
    g_instance->setParent(parent);
  }

  return g_instance;
}

pqSMTKSaveResourceBehavior::~pqSMTKSaveResourceBehavior()
{
  if (g_instance == this)
  {
    g_instance = nullptr;
  }

  QObject::disconnect(this);
}
