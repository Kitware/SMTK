//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKTaskPanel.h"

#include "smtk/extension/qt/task/qtTaskEditor.h"

#include "smtk/extension/paraview/appcomponents/ApplicationConfiguration.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"

#include "smtk/extension/qt/qtDescriptivePhraseModel.h"

#include "smtk/io/Logger.h"

#include "smtk/view/json/jsonView.h"

#include "pqApplicationCore.h"

#include <QVBoxLayout>

pqSMTKTaskPanel::pqSMTKTaskPanel(QWidget* parent)
  : Superclass(parent)
{
  this->setObjectName("pqSMTKTaskPanel");
  auto* pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    pqCore->registerManager("smtk task panel", this);
  }

  // Either we get the application's configuration or we use a default
  // until the application's configuration plugin is loaded.
  bool immediatelyConfigured = false;
  smtk::paraview::ApplicationConfiguration::notify(
    [this, &immediatelyConfigured](smtk::paraview::ApplicationConfiguration& configurator) {
      auto viewInfo = configurator.panelConfiguration(this);
      // Extract just the view configuration.
      auto viewConfig = viewInfo.get<smtk::view::ConfigurationPtr>();
      if (viewConfig)
      {
        this->setView(viewConfig);
        immediatelyConfigured = true;
      }
    });
  if (!immediatelyConfigured)
  {
    // Parse a json representation of our default config, and use it
    // since the application can't immediately configure us.
    smtk::view::ConfigurationPtr config = smtk::extension::qtTaskEditor::defaultConfiguration();
    this->setView(config);
  }

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

pqSMTKTaskPanel::~pqSMTKTaskPanel()
{
  auto* pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    pqCore->unRegisterManager("smtk task panel");
  }
  delete m_viewUIMgr;
  // m_viewUIMgr deletes m_taskPanel
  // deletion of m_taskPanel->widget() is handled when parent widget is deleted.
}

void pqSMTKTaskPanel::setView(const smtk::view::ConfigurationPtr& view)
{
  m_view = view;

  auto* smtkBehavior = pqSMTKBehavior::instance();

  smtkBehavior->visitResourceManagersOnServers([this](pqSMTKWrapper* r, pqServer* s) {
    this->resourceManagerAdded(r, s);
    return false;
  });
}

void pqSMTKTaskPanel::resourceManagerAdded(pqSMTKWrapper* wrapper, pqServer* server)
{
  if (!wrapper || !server)
  {
    return;
  }
  Q_EMIT this->titleChanged("Tasks");

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
    // m_viewUIMgr deletes m_taskPanel, which deletes the container, which deletes child QWidgets.
    m_taskPanel = nullptr;
  }

  m_viewUIMgr = new smtk::extension::qtUIManager(rsrcMgr, viewMgr);
  m_viewUIMgr->setOperationManager(wrapper->smtkOperationManager());
  m_viewUIMgr->setSelection(wrapper->smtkSelection());
  // m_viewUIMgr->setSelectionBit(1);               // ToDo: should be set ?

  smtk::view::Information resinfo;
  resinfo.insert<smtk::view::ConfigurationPtr>(m_view);
  resinfo.insert<QWidget*>(this);
  resinfo.insert<smtk::extension::qtUIManager*>(m_viewUIMgr);

  // the top-level "Type" in m_view should be qtTaskEditor or compatible.
  auto* baseView = m_viewUIMgr->setSMTKView(resinfo);
  m_taskPanel = dynamic_cast<smtk::extension::qtTaskEditor*>(baseView);
  if (baseView && !m_taskPanel)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Unsupported task panel type.");
    return;
  }
  else if (!m_taskPanel)
  {
    return;
  }
  m_taskPanel->widget()->setObjectName("qtTaskEditor");
  std::string title;
  m_view->details().attribute("Title", title);
  if (title.empty())
  {
    title = "Resources";
  }
  this->setWindowTitle(title.c_str());
  Q_EMIT titleChanged(title.c_str());
  if (!m_layout)
  {
    m_layout = new QVBoxLayout;
    m_layout->setObjectName("Layout");
    this->setLayout(m_layout);
  }
  m_layout->addWidget(m_taskPanel->widget());
}

void pqSMTKTaskPanel::resourceManagerRemoved(pqSMTKWrapper* mgr, pqServer* server)
{
  if (!mgr || !server)
  {
    return;
  }
}
