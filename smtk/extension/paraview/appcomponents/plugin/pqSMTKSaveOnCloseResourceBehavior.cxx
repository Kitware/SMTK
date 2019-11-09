//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/plugin/pqSMTKSaveOnCloseResourceBehavior.h"

#include "smtk/common/CompilerInformation.h"

#ifdef SMTK_MSVC
// Ignore "... usage of 'QObject::connect' requires the compiler to capture 'this'..." warnings.
#pragma warning(disable : 4573)
#endif

// Client side
#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqCoreUtilities.h"
#include "pqFileDialog.h"
#include "pqMainWindowEventManager.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqServer.h"
#include "vtkSMProxy.h"
#include "vtkSMProxyManager.h"

#include "smtk/extension/paraview/appcomponents/plugin/pqSMTKSaveResourceBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"
#include "smtk/resource/Manager.h"

#include <QApplication>
#include <QCloseEvent>

static pqSMTKSaveOnCloseResourceBehavior* g_instance = nullptr;

pqSMTKSaveOnCloseResourceBehavior::pqSMTKSaveOnCloseResourceBehavior(QObject* parent)
  : Superclass(parent)
{
  // Wait until the event loop starts, ensuring that the main window will be
  // accessible.
  QTimer::singleShot(0, this, []() {
    // Blech: pqApplicationCore doesn't have the selection manager yet,
    // so wait until we hear that the server is ready to make the connection.
    // We can't have a selection before the first connection, anyway.
    auto pqCore = pqApplicationCore::instance();
    if (pqCore)
    {
      // The first functor listens to the object builder's "destroying" signal
      // to identify pqPipelineSources that are being removed in order to give
      // the user a chance to save resources before they are destroyed. ParaView
      // does not offer us the ability to cancel the destruction of the pipeline
      // source, so we do not offer the option to cancel in our modal dialog.
      pqObjectBuilder* builder = pqCore->getObjectBuilder();
      QObject::connect(builder,
        (void (pqObjectBuilder::*)(pqPipelineSource*)) & pqObjectBuilder::destroying,
        [](pqPipelineSource* source) {
          pqSMTKResource* smtkResource = dynamic_cast<pqSMTKResource*>(source);
          if (smtkResource == nullptr)
          {
            return;
          }

          smtk::resource::ResourcePtr resource = smtkResource->getResource();
          if (resource && resource->clean() == false)
          {
            QMessageBox msgBox;
            msgBox.setText("The resource has been modified.");
            msgBox.setInformativeText("Do you want to save your changes?");
            msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard);
            msgBox.setDefaultButton(QMessageBox::Save);

            int ret = msgBox.exec();

            if (ret == QMessageBox::Save)
            {
              pqActiveObjects* activeObjects = &pqActiveObjects::instance();
              activeObjects->setActiveSource(smtkResource);
              pqSaveResourceReaction::saveResource();
            }
          }
        });

      // The second functor is connected to the "removingManagerFromServer"
      // signal of pqSMTKBehavior, and it checks all resources on the server and
      // prompts the user to save them. Again, we are not offered the ability to
      // cancel whatever action is removing the manager from the server. All we
      // can do is prompt the user to save any modified resources held by the
      // server.
      auto onRemovingManagerFromServer = [&](pqSMTKWrapper* wrapper, pqServer*) {
        std::size_t numberOfUnsavedResources = 0;
        if (!wrapper)
        {
          return false;
        }
        wrapper->visitResources([&](pqSMTKResource* smtkResource) {
          smtk::resource::ResourcePtr resource = smtkResource->getResource();
          if (resource && resource->clean() == false)
          {
            ++numberOfUnsavedResources;
          }
          return false;
        });
        if (numberOfUnsavedResources > 0)
        {
          QMessageBox msgBox;
          if (numberOfUnsavedResources == 1)
          {
            msgBox.setText("The resource has been modified.");
          }
          else
          {
            msgBox.setText(
              QString::number(numberOfUnsavedResources) + " resources have been modified.");
          }
          msgBox.setInformativeText("Do you want to save your changes?");
          msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard);
          msgBox.setDefaultButton(QMessageBox::Save);

          int ret = msgBox.exec();

          if (ret == QMessageBox::Save)
          {
            wrapper->visitResources([&](pqSMTKResource* smtkResource) {
              pqActiveObjects* activeObjects = &pqActiveObjects::instance();
              activeObjects->setActiveSource(smtkResource);
              pqSaveResourceReaction::saveResource();
              return false;
            });
          }
        }
        return false;
      };

      QObject::connect(pqSMTKBehavior::instance(),
        (void (pqSMTKBehavior::*)(pqSMTKWrapper*, pqServer*)) &
          pqSMTKBehavior::removingManagerFromServer,
        onRemovingManagerFromServer);

      // The final functor is connected to the main window's "close" signal, and
      // it prompts the user to save all unsaved resources on all servers. It
      // also provides the user with the ability to cancel the close.
      QObject::connect(pqApplicationCore::instance()->getMainWindowEventManager(),
        &pqMainWindowEventManager::close, [](QCloseEvent* closeEvent) {
          std::size_t numberOfUnsavedResources = 0;
          pqSMTKBehavior::instance()->visitResourceManagersOnServers(
            [&numberOfUnsavedResources](pqSMTKWrapper* wrapper, pqServer*) {
              if (!wrapper)
              {
                return false;
              }
              wrapper->visitResources([&numberOfUnsavedResources](pqSMTKResource* smtkResource) {
                smtk::resource::ResourcePtr resource = smtkResource->getResource();
                if (resource && resource->clean() == false)
                {
                  ++numberOfUnsavedResources;
                }
                return false;
              });
              return false;
            });

          int ret = QMessageBox::Discard;
          if (numberOfUnsavedResources > 0)
          {
            QMessageBox msgBox;
            if (numberOfUnsavedResources == 1)
            {
              msgBox.setText("A resource has been modified.");
            }
            else
            {
              msgBox.setText(
                QString::number(numberOfUnsavedResources) + " resources have been modified.");
            }
            msgBox.setInformativeText("Do you want to save your changes?");
            msgBox.setStandardButtons(
              QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Save);

            ret = msgBox.exec();

            if (ret == QMessageBox::Save)
            {
              pqSMTKBehavior::instance()->visitResourceManagersOnServers(
                [&](pqSMTKWrapper* wrapper, pqServer*) {
                  wrapper->visitResources([&](pqSMTKResource* smtkResource) {
                    pqActiveObjects* activeObjects = &pqActiveObjects::instance();
                    activeObjects->setActiveSource(smtkResource);
                    pqSaveResourceReaction::State state = pqSaveResourceReaction::saveResource();
                    // If the user first selects "Save", is provided a
                    // "Save As" window and then cancels the save, we treat
                    // the action as though the user had pressed "Cancel" at
                    // the first step. This behavior is in keeping with other
                    // document-based applications.
                    if (state == pqSaveResourceReaction::State::Aborted)
                    {
                      ret = QMessageBox::Cancel;
                      return true;
                    }
                    return false;
                  });
                  return ret == QMessageBox::Cancel;
                });
            }
          }
          closeEvent->setAccepted(ret != QMessageBox::Cancel);
        });
    }
  });
}

pqSMTKSaveOnCloseResourceBehavior* pqSMTKSaveOnCloseResourceBehavior::instance(QObject* parent)
{
  if (!g_instance)
  {
    g_instance = new pqSMTKSaveOnCloseResourceBehavior(parent);
  }

  if (g_instance->parent() == nullptr && parent)
  {
    g_instance->setParent(parent);
  }

  return g_instance;
}

pqSMTKSaveOnCloseResourceBehavior::~pqSMTKSaveOnCloseResourceBehavior()
{
  if (g_instance == this)
  {
    g_instance = nullptr;
  }

  QObject::disconnect(this);
}
