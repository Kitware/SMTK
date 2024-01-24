//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKDiagramPanel.h"

#include "smtk/extension/qt/diagram/qtDiagram.h"
#include "smtk/extension/qt/diagram/qtTaskEditor.h"

#include "smtk/extension/paraview/appcomponents/ApplicationConfiguration.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"

#include "smtk/extension/qt/qtDescriptivePhraseModel.h"

#include "smtk/io/Logger.h"

#include "smtk/view/json/jsonView.h"

#include "pqApplicationCore.h"

#include <QAction>
#include <QActionGroup>
#include <QDockWidget>
#include <QLabel>
#include <QLayout>
#include <QToolBar>

pqSMTKDiagramPanel::pqSMTKDiagramPanel(QWidget* parent)
  : Superclass(parent)
{
  this->setObjectName("pqSMTKDiagramPanel");
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

pqSMTKDiagramPanel::~pqSMTKDiagramPanel()
{
  auto* pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    pqCore->unRegisterManager("smtk task panel");
  }
  delete m_viewUIMgr;
  // m_viewUIMgr deletes m_diagram
  // deletion of m_diagram->widget() is handled when parent widget is deleted.
}

void pqSMTKDiagramPanel::setView(const smtk::view::ConfigurationPtr& view)
{
  m_view = view;

  auto* smtkBehavior = pqSMTKBehavior::instance();

  smtkBehavior->visitResourceManagersOnServers([this](pqSMTKWrapper* r, pqServer* s) {
    this->resourceManagerAdded(r, s);
    return false;
  });
}

void pqSMTKDiagramPanel::resourceManagerAdded(pqSMTKWrapper* wrapper, pqServer* server)
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
    // m_viewUIMgr deletes m_diagram, which deletes the container, which deletes child QWidgets.
    m_diagram = nullptr;
  }

  // Add the panel to the View Manger
  viewMgr->elementStateMap()[this->elementType()] = this;

  m_viewUIMgr = new smtk::extension::qtUIManager(rsrcMgr, viewMgr);
  m_viewUIMgr->setOperationManager(wrapper->smtkOperationManager());
  m_viewUIMgr->setSelection(wrapper->smtkSelection());
  // m_viewUIMgr->setSelectionBit(1);               // ToDo: should be set ?

  smtk::view::Information resinfo;
  resinfo.insert<smtk::view::ConfigurationPtr>(m_view);
  resinfo.insert<QWidget*>(this);
  resinfo.insert<smtk::extension::qtUIManager*>(m_viewUIMgr);
  resinfo.insert(wrapper->smtkManagersPtr());

  // the top-level "Type" in m_view should be qtTaskEditor or compatible.
  auto* baseView = m_viewUIMgr->setSMTKView(resinfo);
  m_diagram = dynamic_cast<smtk::extension::qtDiagram*>(baseView);
  if (baseView && !m_diagram)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Unsupported task panel type.");
    return;
  }
  else if (!m_diagram)
  {
    return;
  }
  m_diagram->widget()->setObjectName("qtDiagramView");
  std::string title;
  m_view->details().attribute("Title", title);
  if (title.empty())
  {
    title = "Tasks";
  }
  this->setWindowTitle(title.c_str());
  Q_EMIT titleChanged(title.c_str());
  if (!m_layout)
  {
    m_layout = new QVBoxLayout;
    m_layout->setObjectName("Layout");
    this->setLayout(m_layout);
  }
  m_layout->addWidget(m_diagram->widget());
}

void pqSMTKDiagramPanel::resourceManagerRemoved(pqSMTKWrapper* mgr, pqServer* server)
{
  if (!mgr || !server)
  {
    return;
  }
  smtk::view::ManagerPtr viewMgr = mgr->smtkViewManager();
  if (!viewMgr)
  {
    return;
  }

  // Remove the panel to the View Manger
  auto& stateMap = viewMgr->elementStateMap();
  auto it = stateMap.find(this->elementType());
  if (it != stateMap.end())
  {
    stateMap.erase(it);
  }
}

nlohmann::json pqSMTKDiagramPanel::configuration()
{
  // Simply pass the diagram's configuration along.
  // In the future, we could add to it.
  // One example would be the panel's dock location and size
  // within the main window.
  return m_diagram->configuration();
}

bool pqSMTKDiagramPanel::configure(const nlohmann::json& data)
{
  if (m_diagram)
  {
    // Pass all the configuration data to the diagram.
    // In the future, we may wish to handle some ourselves,
    // such as the panel's dock location and size within
    // the main window (which is not relevant to the diagram).
    return m_diagram->configure(data);
  }
  return false;
}
