//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKResourcePanel.h"

#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResourceBrowser.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"

#include "smtk/extension/qt/qtDescriptivePhraseModel.h"

#include "smtk/view/ResourcePhraseModel.h"
#include "smtk/view/json/jsonView.h"

// cmake puts the .json file contents into a static string, named _xml
#include "smtk/extension/paraview/appcomponents/ResourcePanelConfiguration_xml.h"

pqSMTKResourcePanel::pqSMTKResourcePanel(QWidget* parent)
  : Superclass(parent)
  , m_viewUIMgr(nullptr)
  , m_browser(nullptr)
{
  // Parse a json representation of our default config, save it.
  nlohmann::json j = nlohmann::json::parse(ResourcePanelConfiguration_xml);
  m_view = j[0];

  // Retrieve the ViewManager from the wrapper, and trigger the default browser.
  auto smtkBehavior = pqSMTKBehavior::instance();

  smtkBehavior->visitResourceManagersOnServers([this](pqSMTKWrapper* r, pqServer* s) {
    this->resourceManagerAdded(r, s);
    return false;
  });
  // Now listen for future connections.
  QObject::connect(smtkBehavior, SIGNAL(addedManagerOnServer(pqSMTKWrapper*, pqServer*)), this,
    SLOT(resourceManagerAdded(pqSMTKWrapper*, pqServer*)));
  QObject::connect(smtkBehavior, SIGNAL(removingManagerFromServer(pqSMTKWrapper*, pqServer*)), this,
    SLOT(resourceManagerRemoved(pqSMTKWrapper*, pqServer*)));
}

pqSMTKResourcePanel::~pqSMTKResourcePanel()
{
  delete m_browser;
  // deletion of m_browser->widget() is handled when parent widget is deleted.
}

void pqSMTKResourcePanel::resourceManagerAdded(pqSMTKWrapper* wrapper, pqServer* server)
{
  if (!wrapper || !server)
  {
    return;
  }

  smtk::resource::ManagerPtr rsrcMgr = wrapper->smtkResourceManager();
  smtk::view::ManagerPtr viewMgr = wrapper->smtkViewManager();
  if (!rsrcMgr || !viewMgr)
  {
    return;
  }
  if (m_viewUIMgr)
  {
    delete m_viewUIMgr;
    // m_browser needs to take care of child widgets.
    // while (QWidget* w = this->widget()->findChild<QWidget*>())
    // {
    //   delete w;
    // }
    delete m_browser;
  }

  m_viewUIMgr = new smtk::extension::qtUIManager(rsrcMgr, viewMgr);
  m_viewUIMgr->setOperationManager(wrapper->smtkOperationManager());
  m_viewUIMgr->setSelection(wrapper->smtkSelection());
  // m_viewUIMgr->setSelectionBit(1);               // ToDo: should be set ?

  smtk::extension::ViewInfo resinfo(m_view, this, m_viewUIMgr);
  // NB: We could call
  //     qtSMTKUtilities::registerModelViewConstructor(modelViewName, ...);
  // here to ensure a Qt model-view class in the same plugin is
  // registered before telling the pqSMTKResourceBrowser to use it.

  // TODO handle setting a second time.
  m_browser = new pqSMTKResourceBrowser(resinfo);
  m_browser->widget()->setObjectName("pqSMTKResourceBrowser");
  this->setWindowTitle("Resources");
  this->setWidget(m_browser->widget());
}

void pqSMTKResourcePanel::resourceManagerRemoved(pqSMTKWrapper* mgr, pqServer* server)
{
  if (!mgr || !server)
  {
    return;
  }
}
