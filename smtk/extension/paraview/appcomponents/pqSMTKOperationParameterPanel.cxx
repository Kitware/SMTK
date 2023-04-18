//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKOperationParameterPanel.h"

#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKOperationToolboxPanel.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKRenderResourceBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"

#include "smtk/extension/qt/qtOperationTypeModel.h"
#include "smtk/extension/qt/qtOperationView.h"

#include "smtk/operation/Operation.h"

#include "smtk/view/AvailableOperations.h"
#include "smtk/view/Configuration.h"
#include "smtk/view/Selection.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/operators/Signal.h"

#include "smtk/extension/qt/qtOperationView.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

#include "smtk/workflow/OperationFilterSort.h"
#include "smtk/workflow/json/jsonOperationFilterSort.h"

#include "smtk/io/Logger.h"

#include "smtk/view/Configuration.h"
#include "smtk/view/json/jsonView.h"

#include "pqApplicationCore.h"
#include "pqCoreUtilities.h"
#ifndef PARAVIEW_VERSION_59
#include "pqKeySequences.h"
#include "pqModalShortcut.h"
#endif

#include "vtkSMParaViewPipelineControllerWithRendering.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxy.h"

#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QPointer>
#include <QPushButton>
#include <QTabWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

pqSMTKOperationParameterPanel::pqSMTKOperationParameterPanel(QWidget* parent)
  : Superclass(parent)
{
  this->setWindowTitle("Tool Parameters");
  if (!m_tabs)
  {
    m_tabs = new QTabWidget(this);
    m_tabs->setObjectName("OperationTabs");
    m_tabs->setToolTip(tr("Edit parameters and apply a tool to the current selection."));
    m_tabs->setTabsClosable(true);
    m_tabs->setMovable(true);
    m_tabs->setUsesScrollButtons(true);
    m_layout = new QVBoxLayout;
    m_layout->setObjectName("Layout");
    this->setLayout(m_layout);
    m_layout->addWidget(m_tabs);
    QObject::connect(
      m_tabs, &QTabWidget::tabCloseRequested, this, &pqSMTKOperationParameterPanel::cancelEditing);
  }
  QVBoxLayout* layout = new QVBoxLayout();
  m_tabs->setLayout(layout);

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
    pqCore->registerManager("smtk operation parameters", this);
  }
}

pqSMTKOperationParameterPanel::~pqSMTKOperationParameterPanel() = default;

void pqSMTKOperationParameterPanel::observeWrapper(pqSMTKWrapper* wrapper, pqServer* server)
{
  if (m_wrapper)
  {
    this->unobserveWrapper(m_wrapper, server);
  }
  this->observeToolboxPanels();
  m_wrapper = wrapper;
  // // There is currently no easy way to tell the AvailableOperations instance to
  // // initialize, so for now we simly toggle the "useSelection" choice to
  // // populate the operation panel when a new server is connected.
  // bool useSelection = m_availableOperations->useSelection();
  // m_availableOperations->setUseSelection(!useSelection);
  // m_availableOperations->setUseSelection(useSelection);
}

void pqSMTKOperationParameterPanel::unobserveWrapper(pqSMTKWrapper* wrapper, pqServer* /*unused*/)
{
  if (wrapper != m_wrapper)
  {
    return;
  }

  // Close all open tabs.
  for (; m_tabs->count();)
  {
    this->cancelEditing(0);
  }

  m_wrapper = nullptr;
}

void pqSMTKOperationParameterPanel::runOperationWithDefaults(
  smtk::operation::Operation::Index opType)
{
  if (!m_wrapper)
  {
    smtkInfoMacro(smtk::io::Logger::instance(), "No server connection yet.");
    return;
  }

  auto operation = m_wrapper->smtkOperationManager()->create(opType);
  if (!operation)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Unable to create operation.");
    return;
  }
  auto associations = operation->parameters()->associations();
  auto selection = m_wrapper->smtkSelection();
  if (selection)
  {
    selection->configureItem(associations, /*value*/ 1, /*exactValue*/ false, /*clearItem*/ true);
  }

  this->runOperationWithParameters(operation);
}

void pqSMTKOperationParameterPanel::runOperationWithParameters(
  const std::shared_ptr<smtk::operation::Operation>& operation)
{
  auto* toolboxPanel = qobject_cast<pqSMTKOperationToolboxPanel*>(
    pqApplicationCore::instance()->manager("smtk operation toolbox"));
  if (toolboxPanel)
  {
    auto toolbox = toolboxPanel->toolbox();
    auto* model = toolbox->operationModel();
    model->runOperationWithParameters(operation);
  }
}

void pqSMTKOperationParameterPanel::editExistingOperationParameters(
  const std::shared_ptr<smtk::operation::Operation>& operation)
{
  if (!m_wrapper)
  {
    smtkInfoMacro(smtk::io::Logger::instance(), "No server connection yet.");
    return;
  }
  if (!operation)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "No operation provided.");
    return;
  }

  smtk::operation::Operation::Index opType = operation->index();
  TabData* opTab = nullptr;
  for (auto it = m_views.lower_bound(opType); it != m_views.end() && it->first == opType; ++it)
  {
    // We re-use a tab only if operation pointers match.
    opTab = &it->second;
    if (opTab->m_operation == operation)
    {
      m_tabs->setCurrentWidget(opTab->m_tab);
      this->raise();
      return;
    }
  }

  // We didn't find the operation, so create a new tab.
  auto it = m_views.emplace(opType, TabData{});
  opTab = &it->second;
  opTab->m_operation = operation;
  opTab->m_uiMgr = new smtk::extension::qtUIManager(
    opTab->m_operation, m_wrapper->smtkResourceManager(), m_wrapper->smtkViewManager());
  opTab->m_uiMgr->setMaxValueLabelLength(100);
  opTab->m_uiMgr->managers() = m_wrapper->smtkManagers();
  opTab->m_uiMgr->setOperationManager(m_wrapper->smtkOperationManager());
  opTab->m_uiMgr->setSelection(m_wrapper->smtkSelection());
  opTab->m_tab = new QWidget(m_tabs);
  opTab->m_tab->setLayout(new QVBoxLayout);

  // Fetch the selection and associate it.
  std::set<std::shared_ptr<smtk::resource::PersistentObject>> selected;
  m_wrapper->smtkSelection()->currentSelectionByValue(selected, 1, false);
  auto associations = opTab->m_operation->parameters()->associations();
  if (associations)
  {
    if (associations->isOptional())
    {
      associations->setIsEnabled(!selected.empty());
    }
    if (!selected.empty())
    {
      associations->setNumberOfValues(selected.size());
      associations->setValues(selected.begin(), selected.end());
    }
  }

  smtk::view::ConfigurationPtr view = opTab->m_uiMgr->findOrCreateOperationView();
  opTab->m_view = opTab->m_uiMgr->setSMTKView(view, opTab->m_tab);
  bool didDisplay = opTab->m_view != nullptr;
  if (didDisplay)
  {
    if (auto* opView = dynamic_cast<smtk::extension::qtOperationView*>(opTab->m_view.data()))
    {
      // Override any settings... we run the operation ourselves:
      opView->setRunOperationOnApply(false);
      if (auto doneButton = opView->doneButton())
      {
        doneButton->hide();
      }
      QObject::connect(
        opView,
        &smtk::extension::qtOperationView::operationRequested,
        this,
        &pqSMTKOperationParameterPanel::runOperationWithParameters);
      QObject::connect(
        opView,
        &smtk::extension::qtOperationView::doneEditing,
        this,
        &pqSMTKOperationParameterPanel::cancelTabFromSender);
    }
  }

  m_tabs->insertTab(
    m_tabs->currentIndex() + 1, // Insert after currently-visible operation.
    opTab->m_tab,
    QString::fromStdString(opTab->m_operation->parameters()->type()));
  m_tabs->setCurrentWidget(opTab->m_tab);
  this->raise();
}

void pqSMTKOperationParameterPanel::editOperationParameters(
  smtk::operation::Operation::Index opType)
{
  if (!m_wrapper)
  {
    smtkInfoMacro(smtk::io::Logger::instance(), "No server connection yet.");
    return;
  }
  TabData* opTab = nullptr;
  for (auto it = m_views.lower_bound(opType); it != m_views.end() && it->first == opType; ++it)
  {
    // TODO: Re-use tab only if parameters are default (except perhaps associations).
    // For now, only allow one tab of a given operation type:
    opTab = &it->second;
    m_tabs->setCurrentWidget(opTab->m_tab);
    break;
  }

  // Fetch the selection.
  std::set<std::shared_ptr<smtk::resource::PersistentObject>> selected;
  m_wrapper->smtkSelection()->currentSelectionByValue(selected, 1, false);
  if (opTab && opTab->m_tab)
  {
    m_tabs->setCurrentWidget(opTab->m_tab);

    // Update associations with selection.
    auto associations = opTab->m_operation->parameters()->associations();
    if (associations->isOptional())
    {
      associations->setIsEnabled(!selected.empty());
    }
    if (!selected.empty())
    {
      associations->reset();
      associations->setNumberOfValues(selected.size());
      associations->setValues(selected.begin(), selected.end());
    }
    else if (!associations->isOptional())
    {
      associations->reset();
    }

    auto signalOp = m_wrapper->smtkOperationManager()->create<smtk::attribute::Signal>();
    // TODO: Only run Signal if associations changed.
    signalOp->parameters()
      ->findComponent("modified")
      ->appendValue(opTab->m_operation->parameters());
    signalOp->operate(); // Causes an update of the GUI.
  }
  else
  {
    // Create a new operation and editor tab
    auto it = m_views.emplace(opType, TabData{});
    opTab = &it->second;
    opTab->m_operation = m_wrapper->smtkOperationManager()->create(opType);
    if (!opTab->m_operation)
    {
      smtkErrorMacro(smtk::io::Logger::instance(), "Unable to create operation.");
      return;
    }
    opTab->m_uiMgr = new smtk::extension::qtUIManager(
      opTab->m_operation, m_wrapper->smtkResourceManager(), m_wrapper->smtkViewManager());
    opTab->m_uiMgr->setMaxValueLabelLength(100);
    opTab->m_uiMgr->managers() = m_wrapper->smtkManagers();
    opTab->m_uiMgr->setOperationManager(m_wrapper->smtkOperationManager());
    opTab->m_uiMgr->setSelection(m_wrapper->smtkSelection());
    opTab->m_tab = new QWidget(m_tabs);
    opTab->m_tab->setLayout(new QVBoxLayout);

    // The current tab now has the operation of interest;
    // populate its associations with the current selection.
    auto associations = opTab->m_operation->parameters()->associations();
    if (associations)
    {
      if (associations->isOptional())
      {
        associations->setIsEnabled(!selected.empty());
      }
      if (!selected.empty())
      {
        associations->setNumberOfValues(selected.size());
        associations->setValues(selected.begin(), selected.end());
      }
    }

    smtk::view::ConfigurationPtr view = opTab->m_uiMgr->findOrCreateOperationView();
    opTab->m_view = opTab->m_uiMgr->setSMTKView(view, opTab->m_tab);
    bool didDisplay = opTab->m_view != nullptr;
    if (didDisplay)
    {
      if (auto* opView = dynamic_cast<smtk::extension::qtOperationView*>(opTab->m_view.data()))
      {
        // Override any settings... we run the operation ourselves:
        opView->setRunOperationOnApply(false);
        if (auto doneButton = opView->doneButton())
        {
          doneButton->hide();
        }
        QObject::connect(
          opView,
          &smtk::extension::qtOperationView::operationRequested,
          this,
          &pqSMTKOperationParameterPanel::runOperationWithParameters);
        QObject::connect(
          opView,
          &smtk::extension::qtOperationView::doneEditing,
          this,
          &pqSMTKOperationParameterPanel::cancelTabFromSender);
      }
    }

    m_tabs->insertTab(
      m_tabs->currentIndex() + 1, // Insert after currently-visible operation.
      opTab->m_tab,
      QString::fromStdString(opTab->m_operation->parameters()->type()));
    m_tabs->setCurrentWidget(opTab->m_tab);
  }

  if (opTab)
  {
    if (auto* pw = qobject_cast<QWidget*>(this->parent()))
    {
      pw->show();
      pw->raise();
    }
    opTab->m_tab->show();
    opTab->m_tab->raise();
  }
}

void pqSMTKOperationParameterPanel::cancelEditing(int tabIndex)
{
  QWidget* w = m_tabs->widget(tabIndex);
  if (!w)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Attempt to cancel non-existent operation at index " << tabIndex);
    return;
  }
  for (const auto& editor : m_views)
  {
    if (editor.second.m_tab == w)
    {
      editor.second.m_tab->hide();
      delete editor.second.m_uiMgr;
      while (QWidget* w = editor.second.m_tab->findChild<QWidget*>())
      {
        delete w;
      }
      delete editor.second.m_view;
      m_tabs->removeTab(tabIndex);
      m_views.erase(editor.first);
      return;
    }
  }
  smtkErrorMacro(smtk::io::Logger::instance(), "Unable to find operation for tab " << tabIndex);
}

void pqSMTKOperationParameterPanel::cancelTabFromSender()
{
  auto* viewToCancel = dynamic_cast<smtk::extension::qtOperationView*>(this->sender());
  if (!viewToCancel)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Could not find operation to close.");
    return;
  }
  for (const auto& editor : m_views)
  {
    if (editor.second.m_view == viewToCancel)
    {
      editor.second.m_tab->hide();
      delete editor.second.m_uiMgr;
      while (QWidget* w = editor.second.m_tab->findChild<QWidget*>())
      {
        delete w;
      }
      delete editor.second.m_view;
      m_tabs->removeTab(m_tabs->indexOf(editor.second.m_tab));
      std::cout << m_views.erase(editor.first) << " > 0?\n";
      return;
    }
  }
  smtkErrorMacro(
    smtk::io::Logger::instance(),
    "Unable to find operation for view " << viewToCancel->objectName().toStdString());
}

void pqSMTKOperationParameterPanel::observeToolboxPanels()
{
  // Depending on the order things are initialized, this panel might
  // exist before the toolbox. If so, this lambda will be called
  // after the event loop starts.
  auto connectToolbox = [this]() {
    // Unhook any pre-existing connection to a pqSMTKOperationToolboxPanel
    this->disconnect(this, SLOT(editOperationParameters(smtk::operation::Operation::Index)));
    auto* toolboxPanel = qobject_cast<pqSMTKOperationToolboxPanel*>(
      pqApplicationCore::instance()->manager("smtk operation toolbox"));
    if (toolboxPanel)
    {
      auto toolbox = toolboxPanel->toolbox();
      if (toolbox)
      {
        auto* model = toolbox->operationModel();
        QObject::connect(
          model,
          &qtOperationTypeModel::runOperation,
          this,
          &pqSMTKOperationParameterPanel::runOperation);
        QObject::connect(
          model,
          &qtOperationTypeModel::editOperationParameters,
          this,
          &pqSMTKOperationParameterPanel::editOperationParameters);
      }
    }
  };
  auto* toolboxPanel = qobject_cast<pqSMTKOperationToolboxPanel*>(
    pqApplicationCore::instance()->manager("smtk operation toolbox"));
  if (toolboxPanel && toolboxPanel->toolbox())
  {
    // If we found the panel immediately, hook it up immediately:
    connectToolbox();
  }
  else
  {
    // Wait for the event loop.
    QTimer::singleShot(0, connectToolbox);
  }
}
