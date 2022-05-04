//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKCloseWithActiveOperationBehavior.h"

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

static pqSMTKCloseWithActiveOperationBehavior* g_instance = nullptr;

static std::atomic<int> g_numberOfActiveOperations(0);

pqSMTKCloseWithActiveOperationBehavior::pqSMTKCloseWithActiveOperationBehavior(QObject* parent)
  : Superclass(parent)
{
  // Whenever a new server is connected, attach an operation observer that
  // tracks the number of active operations.
  QObject::connect(
    pqSMTKBehavior::instance(),
    (void (pqSMTKBehavior::*)(pqSMTKWrapper*, pqServer*)) & pqSMTKBehavior::addedManagerOnServer,
    this,
    &pqSMTKCloseWithActiveOperationBehavior::trackActiveOperations);

  // Wait until the event loop starts, ensuring that the main window will be
  // accessible.
  auto* pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    QObject::connect(pqCore, &pqApplicationCore::clientEnvironmentDone, [this]() {
      // This functor is connected to the main window's "close" signal, and
      // it allows the user to cancel the close if there is an active operation.
      QObject::connect(
        pqApplicationCore::instance()->getMainWindowEventManager(),
        &pqMainWindowEventManager::close,
        [](QCloseEvent* closeEvent) {
          int ret = QMessageBox::Close;
          if (g_numberOfActiveOperations > 0)
          {
            ret = pqSMTKCloseWithActiveOperationBehavior::showDialog(g_numberOfActiveOperations);
          }
          if (ret != QMessageBox::Close)
          {
            closeEvent->ignore();
          }
        });
    });
  }
}

void pqSMTKCloseWithActiveOperationBehavior::trackActiveOperations(
  pqSMTKWrapper* wrapper,
  pqServer* server)
{
  (void)server;

  if (!wrapper)
  {
    return;
  }

  m_weakManager = wrapper->smtkOperationManager();

  m_key = wrapper->smtkOperationManager()->observers().insert(
    [](
      const smtk::operation::Operation&,
      smtk::operation::EventType event,
      smtk::operation::Operation::Result) -> int {
      if (event == smtk::operation::EventType::WILL_OPERATE)
      {
        ++g_numberOfActiveOperations;
      }
      else if (event == smtk::operation::EventType::DID_OPERATE)
      {
        --g_numberOfActiveOperations;
      }
      return 0;
    });
}

int pqSMTKCloseWithActiveOperationBehavior::showDialog(std::size_t numberOfActiveOperations)
{
  QMessageBox msgBox;
  if (numberOfActiveOperations == 1)
  {
    msgBox.setText("There is an active operation.");
  }
  else
  {
    msgBox.setText(
      "There are " + QString::number(numberOfActiveOperations) + " active operations.");
  }
  msgBox.setInformativeText("Are you sure you want to close?");
  msgBox.setStandardButtons(QMessageBox::Close | QMessageBox::Cancel);
  msgBox.setDefaultButton(QMessageBox::Cancel);

  int ret = msgBox.exec();
  return ret;
}

pqSMTKCloseWithActiveOperationBehavior* pqSMTKCloseWithActiveOperationBehavior::instance(
  QObject* parent)
{
  if (!g_instance)
  {
    g_instance = new pqSMTKCloseWithActiveOperationBehavior(parent);
  }

  if (g_instance->parent() == nullptr && parent)
  {
    g_instance->setParent(parent);
  }

  return g_instance;
}

pqSMTKCloseWithActiveOperationBehavior::~pqSMTKCloseWithActiveOperationBehavior()
{
  if (g_instance == this)
  {
    g_instance = nullptr;
  }

  auto manager = m_weakManager.lock();
  if (manager != nullptr)
  {
    manager->observers().erase(m_key);
  }

  QObject::disconnect(this);
}
