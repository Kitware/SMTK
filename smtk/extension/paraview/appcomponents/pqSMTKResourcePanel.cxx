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

#include "smtk/extension/paraview/appcomponents/pqSMTKResourceBrowser.h"

#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"

#include "smtk/extension/qt/qtDescriptivePhraseModel.h"

#include "smtk/io/Logger.h"

#include "smtk/view/ResourcePhraseModel.h"
#include "smtk/view/json/jsonView.h"

pqSMTKResourcePanel::pqSMTKResourcePanel(QWidget* parent)
  : Superclass(parent)
  , m_browser(nullptr)
  , m_viewUIMgr(nullptr)
{
  // Parse a json representation of our default config, save it.
  nlohmann::json j = nlohmann::json::parse(pqSMTKResourceBrowser::getJSONConfiguration());
  smtk::view::ConfigurationPtr config = j[0];
  this->setView(config);

  auto* smtkBehavior = pqSMTKBehavior::instance();
  // Now listen for future connections.
  QObject::connect(
    smtkBehavior,
    SIGNAL(addedManagerOnServer(pqSMTKWrapper*, pqServer*)),
    this,
    SLOT(resourceManagerAdded(pqSMTKWrapper*, pqServer*)));
  QObject::connect(
    smtkBehavior,
    SIGNAL(removingManagerFromServer(pqSMTKWrapper*, pqServer*)),
    this,
    SLOT(resourceManagerRemoved(pqSMTKWrapper*, pqServer*)));
}

pqSMTKResourcePanel::~pqSMTKResourcePanel()
{
  delete m_viewUIMgr;
  // m_viewUIMgr deletes m_browser
  // deletion of m_browser->widget() is handled when parent widget is deleted.
}

void pqSMTKResourcePanel::setView(const smtk::view::ConfigurationPtr& view)
{
  m_view = view;

  auto* smtkBehavior = pqSMTKBehavior::instance();

  smtkBehavior->visitResourceManagersOnServers([this](pqSMTKWrapper* r, pqServer* s) {
    this->resourceManagerAdded(r, s);
    return false;
  });
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
    m_viewUIMgr = nullptr;
    // m_viewUIMgr deletes m_browser, which deletes the container, which deletes child QWidgets.
    m_browser = nullptr;
  }

  m_viewUIMgr = new smtk::extension::qtUIManager(rsrcMgr, viewMgr);
  m_viewUIMgr->setOperationManager(wrapper->smtkOperationManager());
  m_viewUIMgr->setSelection(wrapper->smtkSelection());
  // m_viewUIMgr->setSelectionBit(1);               // ToDo: should be set ?

  smtk::extension::ViewInfo resinfo(m_view, this, m_viewUIMgr);

  // the top-level "Type" in m_view should be pqSMTKResourceBrowser or compatible.
  auto* baseview = m_viewUIMgr->setSMTKView(resinfo);
  m_browser = dynamic_cast<pqSMTKResourceBrowser*>(baseview);
  if (!m_browser)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Unsupported resource browser type.");
    return;
  }
  m_browser->widget()->setObjectName("pqSMTKResourceBrowser");
  std::string title;
  m_view->details().attribute("Title", title);
  this->setWindowTitle(title.empty() ? "Resources" : title.c_str());
  this->setWidget(m_browser->widget());
}

void pqSMTKResourcePanel::resourceManagerRemoved(pqSMTKWrapper* mgr, pqServer* server)
{
  if (!mgr || !server)
  {
    return;
  }
}
