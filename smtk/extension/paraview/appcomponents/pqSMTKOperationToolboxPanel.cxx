//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKOperationToolboxPanel.h"

#include "smtk/extension/paraview/appcomponents/ApplicationConfiguration.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKRenderResourceBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"

#include "smtk/operation/Operation.h"

#include "smtk/view/Configuration.h"
#include "smtk/view/OperationDecorator.h"
#include "smtk/view/Selection.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/extension/qt/qtOperationView.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

#include "smtk/workflow/OperationFilterSort.h"
#include "smtk/workflow/json/jsonOperationFilterSort.h"

#include "smtk/io/Logger.h"

#include "smtk/view/Configuration.h"
#include "smtk/view/json/jsonView.h"

#include "pqApplicationCore.h"
#ifndef PARAVIEW_VERSION_59
#include "pqKeySequences.h"
#include "pqModalShortcut.h"
#endif

#include "vtkSMParaViewPipelineControllerWithRendering.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxy.h"

#include <QLineEdit>
#include <QListWidget>
#include <QPointer>
#include <QVBoxLayout>
#include <QWidget>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::shared_ptr<smtk::view::OperationDecorator> pqSMTKOperationToolboxPanel::s_defaultOperations;

pqSMTKOperationToolboxPanel::pqSMTKOperationToolboxPanel(QWidget* parent)
  : Superclass(parent)
{
  this->setWindowTitle("Toolbox");
  QVBoxLayout* layout = new QVBoxLayout();
  this->setLayout(layout);

  // Default configuration.
  nlohmann::json jsonConfig = {
    { "Name", "Operations" },
    { "Type", "qtOperationPalette" },
    { "Component",
      { { "Name", "Details" },
        { "Attributes", { { "SearchBar", true }, { "Title", "Tools" } } },
        { "Children", { { { "Name", "Model" }, { "Attributes", { { "Autorun", "true" } } } } } } } }
  };
  m_configuration = jsonConfig;

  auto* behavior = pqSMTKBehavior::instance();
  QObject::connect(
    behavior,
    SIGNAL(addedManagerOnServer(pqSMTKWrapper*, pqServer*)),
    this,
    SLOT(observeWrapper(pqSMTKWrapper*, pqServer*)));
  QObject::connect(
    behavior,
    SIGNAL(removingManagerFromServer(pqSMTKWrapper*, pqServer*)),
    this,
    SLOT(unobserveWrapper(pqSMTKWrapper*, pqServer*)));
  // Initialize with current wrapper(s), if any:
  behavior->visitResourceManagersOnServers([this](pqSMTKWrapper* wrapper, pqServer* server) {
    this->observeWrapper(wrapper, server);
    return false; // terminate early
  });

  auto* pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    pqCore->registerManager("smtk operation toolbox", this);
  }

  smtk::paraview::ApplicationConfiguration::notify(
    [this](smtk::paraview::ApplicationConfiguration& configurator) {
      auto viewInfo = configurator.panelConfiguration(this);
      this->setConfiguration(viewInfo);
    });
}

void pqSMTKOperationToolboxPanel::setDefaultOperations(
  const std::shared_ptr<smtk::view::OperationDecorator>& decorator)
{
  s_defaultOperations = decorator;
}

pqSMTKOperationToolboxPanel::~pqSMTKOperationToolboxPanel()
{
  delete m_uiMgr;
}

void pqSMTKOperationToolboxPanel::observeWrapper(pqSMTKWrapper* wrapper, pqServer* server)
{
  if (m_view && m_wrapper)
  {
    this->unobserveWrapper(m_wrapper, server);
  }
  m_wrapper = wrapper;
  if (m_wrapper)
  {
    this->reconfigure();
  }
  else
  {
    // this->m_availableOperations->setSelection(nullptr);
    // this->m_availableOperations->setOperationManager(nullptr);
    m_view = nullptr;
    m_uiMgr = nullptr;
  }

  // // There is currently no easy way to tell the AvailableOperations instance to
  // // initialize, so for now we simly toggle the "useSelection" choice to
  // // populate the operation panel when a new server is connected.
  // bool useSelection = m_availableOperations->useSelection();
  // m_availableOperations->setUseSelection(!useSelection);
  // m_availableOperations->setUseSelection(useSelection);
}

void pqSMTKOperationToolboxPanel::unobserveWrapper(pqSMTKWrapper* wrapper, pqServer* /*unused*/)
{
  if (wrapper != m_wrapper)
  {
    return;
  }

  if (m_view)
  {
    // Empty the existing view.
    while (QWidget* w = this->findChild<QWidget*>())
    {
      delete w;
    }
    // delete m_view;
    // delete m_uiMgr;
    m_view = nullptr;
    m_uiMgr = nullptr;
  }

  // if (wrapper)
  // {
  //   this->m_availableOperations->setSelection(nullptr);
  //   this->m_availableOperations->setOperationManager(nullptr);
  // }
  m_wrapper = nullptr;
}

void pqSMTKOperationToolboxPanel::searchFocus()
{
  if (m_view)
  {
    if (auto* searchTextWidget = m_view->searchTextWidget())
    {
      this->activateWindow();
      this->raise();
      searchTextWidget->setFocus(Qt::ShortcutFocusReason);
    }
  }
}

bool pqSMTKOperationToolboxPanel::setConfiguration(const smtk::view::Information& info)
{
  if (!info.contains<std::shared_ptr<smtk::view::Configuration>>())
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "No view configuration provided.");
    return false;
  }
  const auto& config = info.get<std::shared_ptr<smtk::view::Configuration>>();
  if (!config)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "View configuration is empty.");
    return false;
  }

  bool needReconfigure = false;
  if (!(*config == *m_configuration))
  {
    m_configuration = config;
    needReconfigure = true;
  }

  if (info.contains<std::shared_ptr<smtk::view::OperationDecorator>>())
  {
    const auto& operationDecorator = info.get<std::shared_ptr<smtk::view::OperationDecorator>>();
    if (operationDecorator && operationDecorator != s_defaultOperations)
    {
      s_defaultOperations = operationDecorator;
      needReconfigure = true;
    }
  }
  if (needReconfigure)
  {
    this->reconfigure();
  }
  return needReconfigure;
}

void pqSMTKOperationToolboxPanel::reconfigure()
{
  if (!m_wrapper)
  {
    return;
  }

  if (m_view)
  {
    // Empty the existing view.
    while (QWidget* w = this->findChild<QWidget*>())
    {
      delete w;
    }
    // delete m_view;
    // delete m_uiMgr;
    m_view = nullptr;
    m_uiMgr = nullptr;
  }

  m_uiMgr = new smtk::extension::qtUIManager(
    m_wrapper->smtkResourceManager(), m_wrapper->smtkViewManager());
  m_uiMgr->managers() = m_wrapper->smtkManagers();
  m_uiMgr->setOperationManager(m_wrapper->smtkOperationManager());
  auto managers = smtk::common::Managers::create();
  managers->insert_or_assign(m_wrapper->smtkResourceManager());
  managers->insert_or_assign(m_wrapper->smtkOperationManager());
  managers->insert_or_assign(m_wrapper->smtkViewManager());
  managers->insert_or_assign(m_wrapper->smtkSelection());
  if (
    auto decorator =
      m_wrapper->smtkManagers().get<std::shared_ptr<smtk::view::OperationDecorator>>())
  {
    managers->insert_or_assign(decorator);
  }
  else if (s_defaultOperations)
  {
    managers->insert_or_assign(s_defaultOperations);
  }
  smtk::view::Information viewInfo;
  this->setToolTip("Click an operation to edit its parameters.\n"
                   "Double-click an operation to run it with default parameters.");
  QWidget* ww = this;
  viewInfo.insert(ww);
  viewInfo.insert(m_configuration);
  viewInfo.insert(managers);
  viewInfo.insert(m_uiMgr.data());
  auto* view = m_uiMgr->setSMTKView(viewInfo);
  m_view = dynamic_cast<smtk::extension::qtOperationPalette*>(view);
#ifndef PARAVIEW_VERSION_59
  if (m_view && m_view->searchTextWidget())
  {
    if (!m_findOperationShortcut)
    {
      m_findOperationShortcut = pqKeySequences::instance().addModalShortcut(
        QKeySequence(Qt::CTRL + Qt::Key_Space), nullptr, this);
      // Make the shortcut application-wide:
      m_findOperationShortcut->setContextWidget(this, Qt::ApplicationShortcut);
    }
    QObject::connect(
      m_findOperationShortcut,
      &pqModalShortcut::activated,
      this,
      &pqSMTKOperationToolboxPanel::searchFocus);
  }
#endif
}
