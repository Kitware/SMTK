//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKSaveOnCloseResourceBehavior.h"

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
#include "pqSettings.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMSessionProxyManager.h"

#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKSaveResourceBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"
#include "smtk/extension/paraview/server/vtkSMTKSettings.h"
#include "smtk/io/Logger.h"
#include "smtk/resource/Manager.h"

#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>

static pqSMTKSaveOnCloseResourceBehavior* g_instance = nullptr;

pqSMTKSaveOnCloseResourceBehavior::pqSMTKSaveOnCloseResourceBehavior(QObject* parent)
  : Superclass(parent)
{
  auto* pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    QObject::connect(pqCore, &pqApplicationCore::clientEnvironmentDone, [this, pqCore]() {
      // The first functor listens to the object builder's "destroying" signal
      // to identify pqPipelineSources that are being removed in order to give
      // the user a chance to save resources before they are destroyed. ParaView
      // does not offer us the ability to cancel the destruction of the pipeline
      // source, so we do not offer the option to cancel in our modal dialog.
      pqObjectBuilder* builder = pqCore->getObjectBuilder();
      QObject::connect(
        builder,
        (void (pqObjectBuilder::*)(pqPipelineSource*)) & pqObjectBuilder::destroying,
        [](pqPipelineSource* source) {
          pqSMTKResource* smtkResource = dynamic_cast<pqSMTKResource*>(source);
          if (smtkResource == nullptr)
          {
            return;
          }

          smtk::resource::ResourcePtr resource = smtkResource->getResource();
          if (resource && !resource->clean())
          {
            int ret = QMessageBox::Discard;
            ret = pqSMTKSaveOnCloseResourceBehavior::showDialogWithPrefs(1, false);

            if (ret == QMessageBox::Save)
            {
              pqActiveObjects* activeObjects = &pqActiveObjects::instance();
              activeObjects->setActiveSource(smtkResource);
              pqSaveResourceReaction::saveResource(smtkResource);
            }
          }
        });

      // The second functor is connected to the "removingManagerFromServer"
      // signal of pqSMTKBehavior, and it checks all resources on the server and
      // prompts the user to save them. Again, we are not offered the ability to
      // cancel whatever action is removing the manager from the server. All we
      // can do is prompt the user to save any modified resources held by the
      // server.
      auto onRemovingManagerFromServer = [&](pqSMTKWrapper* wrapper, pqServer* /*unused*/) {
        std::size_t numberOfUnsavedResources = 0;
        if (!wrapper)
        {
          return false;
        }
        wrapper->visitResources([&](pqSMTKResource* smtkResource) {
          smtk::resource::ResourcePtr resource = smtkResource->getResource();
          if (resource && !resource->clean())
          {
            ++numberOfUnsavedResources;
          }
          return false;
        });
        if (numberOfUnsavedResources > 0)
        {
          int ret = QMessageBox::Discard;
          ret =
            pqSMTKSaveOnCloseResourceBehavior::showDialogWithPrefs(numberOfUnsavedResources, false);

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

      QObject::connect(
        pqSMTKBehavior::instance(),
        (void (pqSMTKBehavior::*)(pqSMTKWrapper*, pqServer*)) &
          pqSMTKBehavior::removingManagerFromServer,
        onRemovingManagerFromServer);

      // The final functor is connected to the main window's "close" signal, and
      // it prompts the user to save all unsaved resources on all servers. It
      // also provides the user with the ability to cancel the close.
      QObject::connect(
        pqApplicationCore::instance()->getMainWindowEventManager(),
        &pqMainWindowEventManager::close,
        [](QCloseEvent* closeEvent) {
          std::size_t numberOfUnsavedResources = 0;
          pqSMTKBehavior::instance()->visitResourceManagersOnServers(
            [&numberOfUnsavedResources](pqSMTKWrapper* wrapper, pqServer* /*unused*/) {
              if (!wrapper)
              {
                return false;
              }
              wrapper->visitResources([&numberOfUnsavedResources](pqSMTKResource* smtkResource) {
                smtk::resource::ResourcePtr resource = smtkResource->getResource();
                if (resource && !resource->clean())
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
            ret = pqSMTKSaveOnCloseResourceBehavior::showDialogWithPrefs(
              numberOfUnsavedResources, true);

            if (ret == QMessageBox::Save)
            {
              pqSMTKBehavior::instance()->visitResourceManagersOnServers(
                [&](pqSMTKWrapper* wrapper, pqServer* /*unused*/) {
                  wrapper->visitResources([&](pqSMTKResource* smtkResource) {
                    pqActiveObjects* activeObjects = &pqActiveObjects::instance();
                    activeObjects->setActiveSource(smtkResource);
                    pqSaveResourceReaction::State state = pqSaveResourceReaction::saveResource();
                    // If the user first selects "Save", is provided a
                    // "Save As" window and then cancels the save, we treat
                    // the action as though the user had pressed "Cancel" at
                    // the first step. This behavior is in keeping with other
                    // document-based applications.
                    // If the user has set the DontShowAndSave preference,
                    // there's no way to exit the app with a modified resource -
                    // they must explicitly close and discard each modified resource.
                    if (state == pqSaveResourceReaction::State::Aborted)
                    {
                      // explain to the user what's happening if the pref is set.
                      auto* settings = vtkSMTKSettings::GetInstance();
                      int showSave = settings->GetShowSaveResourceOnClose();
                      if (showSave == vtkSMTKSettings::DontShowAndSave)
                      {
                        smtkInfoMacro(
                          smtk::io::Logger::instance(),
                          "Your preference is set to save modified resources when they are closed. "
                          "Please choose \"File .. Close Resource\" and cancel the save if you "
                          "wish to discard changes before exiting.");
                      }
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
    });
  }
}

int pqSMTKSaveOnCloseResourceBehavior::showDialog(
  bool& cbChecked,
  std::size_t numberOfUnsavedResources,
  bool showCancel)
{
  QMessageBox msgBox;
  QCheckBox* cb = new QCheckBox("Set default and don't show again.");
  if (numberOfUnsavedResources == 1)
  {
    msgBox.setText("A resource has been modified.");
  }
  else
  {
    msgBox.setText(QString::number(numberOfUnsavedResources) + " resources have been modified.");
  }
  msgBox.setInformativeText("Do you want to save your changes?");
  auto buttons = QMessageBox::Save | QMessageBox::Discard;
  if (showCancel)
  {
    buttons |= QMessageBox::Cancel;
  }
  msgBox.setStandardButtons(buttons);
  msgBox.setDefaultButton(QMessageBox::Save);
  msgBox.setCheckBox(cb);

  int ret = msgBox.exec();
  cbChecked = cb->isChecked();
  return ret;
}

int pqSMTKSaveOnCloseResourceBehavior::showDialogWithPrefs(
  std::size_t numberOfUnsavedResources,
  bool showCancel)
{
  int ret = QMessageBox::Discard;
  auto* settings = vtkSMTKSettings::GetInstance();
  int showSave = settings->GetShowSaveResourceOnClose();
  bool cbChecked = false;
  if (showSave == vtkSMTKSettings::AskUser)
  {
    ret = pqSMTKSaveOnCloseResourceBehavior::showDialog(
      cbChecked, numberOfUnsavedResources, showCancel);
  }
  else if (showSave == vtkSMTKSettings::DontShowAndSave)
  {
    ret = QMessageBox::Save;
  }
  // only true if messagebox was shown and user checked - save settings.
  if (cbChecked && ret != QMessageBox::Cancel)
  {
    // set preference in settings.
    showSave = ret == QMessageBox::Discard ? vtkSMTKSettings::DontShowAndDiscard
                                           : vtkSMTKSettings::DontShowAndSave;
    // save on server, too, so they persist. From pqWelcomeDialog
    pqServer* server = pqApplicationCore::instance()->getActiveServer();
    if (server)
    {
      vtkSMSessionProxyManager* pxm = server->proxyManager();
      if (pxm)
      {
        vtkSMProxy* proxy = pxm->GetProxy("settings", "SMTKSettings");
        if (proxy)
        {
          const char* pname = "ShowSaveResourceOnClose";
          vtkSMPropertyHelper(proxy, pname).Set(showSave);
          proxy->UpdateVTKObjects();
          // Force a save, otherwise change won't persist across restarts.
          // From pqSettingsDialog::onAccepted()
          pqSettings* qSettings = pqApplicationCore::instance()->settings();
          vtkSMProperty* smproperty = proxy->GetProperty(pname);
          QString key = QString("%1.%2").arg(proxy->GetXMLName()).arg(pname);
          qSettings->saveInQSettings(key.toLocal8Bit().data(), smproperty);
        }
      }
    }
  }

  return ret;
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
