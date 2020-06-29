//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/project/plugin/AutoStart.h"

#include "smtk/extension/paraview/appcomponents/pqSMTKResourcePanel.h"
#include "smtk/project/Project.h"
#include "smtk/project/plugin/pqSMTKProjectMenu.h"
#include "smtk/view/ResourcePhraseModel.h"

#include "pqApplicationCore.h"

#include <QApplication>
#include <QMainWindow>
#include <QTimer>

namespace
{
void setView()
{
  for (QWidget* w : QApplication::topLevelWidgets())
  {
    QMainWindow* mainWindow = dynamic_cast<QMainWindow*>(w);
    if (mainWindow)
    {
      pqSMTKResourcePanel* dock = mainWindow->findChild<pqSMTKResourcePanel*>();
      // If the dock is not there, just try it again.
      if (dock)
      {
        auto phraseModel = std::dynamic_pointer_cast<smtk::view::ResourcePhraseModel>(
          dock->resourceBrowser()->phraseModel());
        if (phraseModel)
        {
          phraseModel->setFilter([](const smtk::resource::Resource& resource) {
            return !resource.isOfType(smtk::common::typeName<smtk::project::Project>());
          });
        }
      }
      else
      {
        QTimer::singleShot(10, []() { setView(); });
      }
    }
  }
}
} // namespace

AutoStart::AutoStart(QObject* parent)
  : Superclass(parent)
{
}

AutoStart::~AutoStart() = default;

void AutoStart::startup()
{
  auto projectMenuMgr = pqSMTKProjectMenu::instance(this);

  auto pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    pqCore->registerManager("smtk project menu", projectMenuMgr);
  }

  // Since the loading order of smtk plugins is indeterminate, a spinning
  // function call is used here to set up the custom view.
  QTimer::singleShot(10, []() { setView(); });
}

void AutoStart::shutdown()
{
  auto pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    pqCore->unRegisterManager("smtk project menu");
  }
}
