//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/project/pqSMTKProjectAutoStart.h"

#include "smtk/extension/paraview/appcomponents/pqSMTKResourceDock.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResourcePanel.h"
#include "smtk/extension/paraview/project/pqSMTKProjectMenu.h"
#include "smtk/extension/paraview/project/pqSMTKTaskResourceVisibility.h"
#include "smtk/project/Project.h"
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
      pqSMTKResourceDock* dock = mainWindow->findChild<pqSMTKResourceDock*>();
      pqSMTKResourcePanel* panel =
        dock ? qobject_cast<pqSMTKResourcePanel*>(dock->widget()) : nullptr;
      // If the dock is not there, just try it again.
      if (panel)
      {
        if (auto* browser = panel->resourceBrowser())
        {
          auto phraseModel =
            std::dynamic_pointer_cast<smtk::view::ResourcePhraseModel>(browser->phraseModel());
          if (phraseModel)
          {
            phraseModel->setFilter([](const smtk::resource::Resource& resource) {
              return !resource.isOfType(smtk::common::typeName<smtk::project::Project>());
            });
          }
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

pqSMTKProjectAutoStart::pqSMTKProjectAutoStart(QObject* parent)
  : Superclass(parent)
{
}

pqSMTKProjectAutoStart::~pqSMTKProjectAutoStart() = default;

void pqSMTKProjectAutoStart::startup()
{
  auto* projectMenuMgr = pqSMTKProjectMenu::instance(this);
  auto* taskResourceVis = pqSMTKTaskResourceVisibility::instance(this);

  auto* pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    pqCore->registerManager("smtk project menu", projectMenuMgr);
    pqCore->registerManager("smtk task visibility", taskResourceVis);
  }

  // Since the loading order of smtk plugins is indeterminate, a spinning
  // function call is used here to set up the custom view.
  QTimer::singleShot(10, []() { setView(); });
}

void pqSMTKProjectAutoStart::shutdown()
{
  auto* pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    pqCore->unRegisterManager("smtk display attribute on load");
    pqCore->unRegisterManager("smtk project menu");
  }
}
