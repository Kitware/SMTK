//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKAttributePanel.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Resource.h"

#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"

#include "smtk/extension/paraview/server/vtkSMTKSettings.h"

#include "smtk/extension/qt/qtBaseView.h"

#include "smtk/io/Logger.h"

#include "smtk/project/Manager.h"
#include "smtk/project/Project.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Observer.h"
#include "smtk/resource/Properties.h"
#include "smtk/resource/Resource.h"

#include "smtk/view/Configuration.h"
#include "smtk/view/Selection.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqCoreUtilities.h"
#include "pqPipelineSource.h"

#include "vtkSMProperty.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxy.h"
#include "vtkSMSessionProxyManager.h"

#include "vtkCommand.h"
#include "vtkVector.h"

#include <QAction>
#include <QDockWidget>
#include <QPointer>
#include <QTimer>
#include <QVBoxLayout>

pqSMTKAttributePanel::pqSMTKAttributePanel(QWidget* parent)
  : Superclass(parent)
{
  this->setObjectName("pqSMTKAttributePanel");
  this->setLayout(new QVBoxLayout);
  this->layout()->setObjectName("pqSMTKAttributePanel_layout");
  this->updateTitle();
  auto* behavior = pqSMTKBehavior::instance();
  QObject::connect(
    behavior,
    SIGNAL(postProcessingModeChanged(bool)),
    this,
    SLOT(displayActivePipelineSource(bool)));
  QObject::connect(
    behavior,
    SIGNAL(addedManagerOnServer(pqSMTKWrapper*, pqServer*)),
    this,
    SLOT(observeProjectsOnServer(pqSMTKWrapper*, pqServer*)));
  QObject::connect(
    behavior,
    SIGNAL(removingManagerFromServer(pqSMTKWrapper*, pqServer*)),
    this,
    SLOT(unobserveProjectsOnServer(pqSMTKWrapper*, pqServer*)));
  behavior->visitResourceManagersOnServers([this](pqSMTKWrapper* wrapper, pqServer* server) {
    this->observeProjectsOnServer(wrapper, server);
    return false; // terminate early
  });
  auto* pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    pqCore->registerManager("smtk attribute panel", this);
  }

  auto* smtkSettings = vtkSMTKSettings::GetInstance();
  pqCoreUtilities::connect(smtkSettings, vtkCommand::ModifiedEvent, this, SLOT(updateSettings()));
}

pqSMTKAttributePanel::~pqSMTKAttributePanel()
{
  if (auto rsrc = m_rsrc.lock())
  {
    auto rsrcMgr = rsrc->manager();
    if (rsrcMgr && m_observer.assigned())
    {
      rsrcMgr->observers().erase(m_observer);
    }
  }

  m_propertyLinks.clear();
  delete m_attrUIMgr;
  m_attrUIMgr = nullptr;
}

bool pqSMTKAttributePanel::displayPipelineSource(pqPipelineSource* psrc)
{
  pqSMTKResource* rsrc = dynamic_cast<pqSMTKResource*>(psrc);
  if (rsrc)
  {
    auto attrRsrc = std::dynamic_pointer_cast<smtk::attribute::Resource>(rsrc->getResource());
    if (attrRsrc && attrRsrc != m_rsrc.lock())
    {
      pqSMTKWrapper* wrapper =
        pqSMTKBehavior::instance()->resourceManagerForServer(rsrc->getServer());
      this->updateManagers(wrapper ? wrapper->smtkManagersPtr() : nullptr);
      return this->displayResource(attrRsrc);
    }
  }
  return false;
}

bool pqSMTKAttributePanel::displayResource(
  const smtk::attribute::ResourcePtr& rsrc,
  smtk::view::ConfigurationPtr view,
  int advancedlevel)
{
  bool didDisplay = false;

  if (rsrc)
  {
    auto previousResource = m_rsrc.lock();

    if (!rsrc->isPrivate() && rsrc != previousResource)
    {
      if (previousResource)
      {
        previousResource->properties().erase<bool>("smtk.attribute_panel.display_hint");
      }
      resetPanel(rsrc->manager());
      didDisplay = displayResourceInternal(rsrc, view, advancedlevel);
    }
    else if (rsrc->isPrivate() && rsrc == previousResource)
    {
      // the panel is displaying a resource that is now private
      // stop displaying it
      resetPanel(rsrc->manager());
    }
  }
  else
  {
    this->resetPanel(nullptr);
    this->updateTitle();
  }

  return didDisplay;
}

bool pqSMTKAttributePanel::displayResourceOnServer(
  const smtk::attribute::ResourcePtr& rsrc,
  smtk::view::ConfigurationPtr view,
  int advancedlevel)
{
  smtk::resource::ManagerPtr rsrcMgr;
  if (rsrc && (rsrcMgr = rsrc->manager()))
  {
    auto* behavior = pqSMTKBehavior::instance();
    pqSMTKWrapper* wrapper = behavior->getPVResourceManager(rsrcMgr);
    this->updateManagers(wrapper ? wrapper->smtkManagersPtr() : nullptr);
    return this->displayResource(rsrc, view, advancedlevel);
  }
  return false;
}

bool pqSMTKAttributePanel::displayView(smtk::view::ConfigurationPtr view)
{
  if (!view)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Null view passed to attribute panel.");
    return false;
  }

  if (!m_attrUIMgr)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "View passed but no resource indicated.");
    return false;
  }
  // It is possible in some cases to display attributes not part of the
  // task workflow. In that case, we need to stop watching the currently-active
  // task (so its completion state changing does not disable an unrelated
  // attribute editor) and force the panel widget to be enabled.
  // If this new view _is_ part of the currently-active task, the enabled
  // status and observer will be reset after this by the method calling us.
  this->m_attrUIMgr->setReadOnly(false);
  if (m_currentTask)
  {
    m_currentTask->observers().erase(m_currentTaskObserverKey);
  }
  m_currentTask = nullptr;
  auto* qview = m_attrUIMgr->setSMTKView(view, this);
  if (!qview)
  {
    return false;
  }

  this->focusPanel();
  return true;
}

bool pqSMTKAttributePanel::updatePipeline()
{
  auto* dataSource = pqActiveObjects::instance().activeSource();
  return this->displayPipelineSource(dataSource);
}

void pqSMTKAttributePanel::resetPanel(smtk::resource::ManagerPtr rsrcMgr)
{
  (void)rsrcMgr;
  if (m_attrUIMgr)
  {
    m_propertyLinks.clear();
    delete m_attrUIMgr;
    m_attrUIMgr = nullptr;
    while (QWidget* w = this->findChild<QWidget*>())
    {
      delete w;
    }
  }

  m_observer.release();
  m_rsrc = std::weak_ptr<smtk::resource::Resource>();
}

void pqSMTKAttributePanel::focusPanel()
{
  // If we are owned by a dock widget, ensure the dock is shown.
  if (auto* dock = qobject_cast<QDockWidget*>(this->parent()))
  {
    auto* action = dock->toggleViewAction();
    if (!action->isChecked())
    {
      action->trigger();
    }
  }
  // Raise our parent widget.
  if (auto* parent = qobject_cast<QWidget*>(this->parent()))
  {
    parent->raise();
  }
}

void pqSMTKAttributePanel::updateSettings()
{
  if (!m_attrUIMgr)
  {
    return;
  }

  auto* smtkSettings = vtkSMTKSettings::GetInstance();
  m_attrUIMgr->setHighlightOnHover(smtkSettings->GetHighlightOnHover());
}

void pqSMTKAttributePanel::displayActivePipelineSource(bool doDisplay)
{
  if (doDisplay)
  {
    QObject::connect(
      &pqActiveObjects::instance(),
      SIGNAL(sourceChanged(pqPipelineSource*)),
      this,
      SLOT(displayPipelineSource(pqPipelineSource*)),
      Qt::QueuedConnection);
    QObject::connect(
      &pqActiveObjects::instance(),
      SIGNAL(dataUpdated()),
      this,
      SLOT(updatePipeline()),
      Qt::QueuedConnection);
  }
  else
  {
    QObject::disconnect(
      &pqActiveObjects::instance(),
      SIGNAL(sourceChanged(pqPipelineSource*)),
      this,
      SLOT(displayPipelineSource(pqPipelineSource*)));
    QObject::disconnect(
      &pqActiveObjects::instance(), SIGNAL(dataUpdated()), this, SLOT(updatePipeline()));
  }
}

void pqSMTKAttributePanel::observeProjectsOnServer(pqSMTKWrapper* mgr, pqServer* server)
{
  (void)server;
  if (!mgr)
  {
    return;
  }
  auto projectManager = mgr->smtkProjectManager();
  if (!projectManager)
  {
    return;
  }

  QPointer<pqSMTKAttributePanel> self(this);
  auto observerKey = projectManager->observers().insert(
    [self](const smtk::project::Project& project, smtk::project::EventType event) {
      if (self)
      {
        self->handleProjectEvent(project, event);
      }
    },
    0,    // assign a neutral priority
    true, // immediatelyNotify
    "pqSMTKAttributePanel: Display active task's related attributes in panel.");
  m_projectManagerObservers[projectManager] = std::move(observerKey);
}

void pqSMTKAttributePanel::unobserveProjectsOnServer(pqSMTKWrapper* mgr, pqServer* server)
{
  (void)server;
  if (!mgr)
  {
    return;
  }
  auto projectManager = mgr->smtkProjectManager();
  if (!projectManager)
  {
    return;
  }

  auto entry = m_projectManagerObservers.find(projectManager);
  if (entry != m_projectManagerObservers.end())
  {
    projectManager->observers().erase(entry->second);
    m_projectManagerObservers.erase(entry);
  }
}

void pqSMTKAttributePanel::handleProjectEvent(
  const smtk::project::Project& project,
  smtk::project::EventType event)
{
  auto* taskManager = const_cast<smtk::task::Manager*>(&project.taskManager());
  QPointer<pqSMTKAttributePanel> self(this);
  switch (event)
  {
    case smtk::project::EventType::ADDED:
      // observe the active task
      // Use QTimer to wait until the event queue is emptied before trying this;
      // that gives operations time to complete. Blech.
      QTimer::singleShot(0, [this, taskManager, self]() {
        if (!self)
        {
          return;
        }
        auto& activeTracker = taskManager->active();
        m_activeObserverKey = activeTracker.observers().insert(
          [this, self](smtk::task::Task* oldTask, smtk::task::Task* newTask) {
            if (!self)
            {
              return;
            }
            // Stop observing the prior task (if any).
            m_currentTaskObserverKey.release();

            (void)oldTask;
            if (m_currentTask)
            {
              m_currentTask->observers().erase(m_currentTaskObserverKey);
              self->displayResource(nullptr);
            }
            m_currentTask = nullptr;
            if (newTask)
            {
              self->displayTaskAttribute(newTask);
            }
          },
          /* priority */ 0,
          /* initialize */ true,
          "AttributePanel active task tracking");
      });
      break;
    case smtk::project::EventType::REMOVED:
      // stop observing active-task tracker and any active task.
      if (m_currentTask)
      {
        m_currentTask->observers().erase(m_currentTaskObserverKey);
      }
      m_currentTask = nullptr;
      m_activeObserverKey.release();
      // Remove our observer key later.
      QTimer::singleShot(0, [this, taskManager, self]() {
        if (!self)
        {
          return;
        }
        auto& activeTracker = taskManager->active();
        activeTracker.observers().erase(m_activeObserverKey);
      });
      break;
    case smtk::project::EventType::MODIFIED:
    default:
      // Do nothing.
      break;
  }
}

bool pqSMTKAttributePanel::updateManagers(const std::shared_ptr<smtk::common::Managers>& managers)
{
  if (!managers)
  {
    m_seln = nullptr;
    m_opManager = nullptr;
    m_viewManager = nullptr;
    return false;
  }
  // Keep hold of the selection instance for the active server connection
  // so that this->displayResource() below can make use of it.
  m_seln = managers->get<smtk::view::Selection::Ptr>();
  m_opManager = managers->get<smtk::operation::Manager::Ptr>();
  m_viewManager = managers->get<smtk::view::Manager::Ptr>();
  return true;
}

bool pqSMTKAttributePanel::displayResourceInternal(
  const smtk::attribute::ResourcePtr& rsrc,
  smtk::view::ConfigurationPtr view,
  int advancedlevel)
{
  bool didDisplay = false;

  m_rsrc = rsrc;

  m_attrUIMgr = new smtk::extension::qtUIManager(rsrc);
  m_attrUIMgr->setOperationManager(m_opManager); // Assign the operation manager
  m_attrUIMgr->setViewManager(m_viewManager);
  m_attrUIMgr->setSelection(m_seln); // NB: m_seln may be null.
  m_attrUIMgr->setSelectionBit(1);   // ToDo: should be set by application

  // Find or Create a value for highlight on hover
  if (m_seln)
  {
    int hoverBit = m_seln->findOrCreateLabeledValue("hovered");
    m_attrUIMgr->setHoverBit(hoverBit);
  }

  // Start watching the resource's associate PV server for user preference changes.
  pqServer* server = pqActiveObjects::instance().activeServer();
  vtkSMSessionProxyManager* pxm = server ? server->proxyManager() : nullptr;
  auto* paletteProxy = pxm ? pxm->GetProxy("settings", "ColorPalette") : nullptr;
  auto* defaultValueColorProp =
    paletteProxy ? paletteProxy->GetProperty("SMTKDefaultValueBackground") : nullptr;
  auto* invalidValueColorProp =
    paletteProxy ? paletteProxy->GetProperty("SMTKInvalidValueBackground") : nullptr;
  if (defaultValueColorProp && invalidValueColorProp)
  {
    vtkVector3d dc;
    vtkSMPropertyHelper(defaultValueColorProp).Get(dc.GetData(), 3);
    QVariantList vdc;
    vdc << dc[0] << dc[1] << dc[2];
    m_attrUIMgr->setDefaultValueColorRgbF(vdc);

    vtkVector3d ic;
    vtkSMPropertyHelper(invalidValueColorProp).Get(ic.GetData(), 3);
    QVariantList vic;
    vic << ic[0] << ic[1] << ic[2];
    m_attrUIMgr->setInvalidValueColorRgbF(vic);

    m_propertyLinks.addPropertyLink(
      m_attrUIMgr,
      "defaultValueColorRgbF",
      SIGNAL(defaultValueColorChanged()),
      paletteProxy,
      defaultValueColorProp);
    m_propertyLinks.addPropertyLink(
      m_attrUIMgr,
      "invalidValueColorRgbF",
      SIGNAL(invalidValueColorChanged()),
      paletteProxy,
      invalidValueColorProp);
  }

  // Fetch the current user preferences and update the UI manager with them.
  this->updateSettings();

  // Was the view specified or are we using the Resource's TopLevel View?
  smtk::view::ConfigurationPtr theView = view ? view : (rsrc ? rsrc->findTopLevelView() : nullptr);
  if (theView)
  {
    didDisplay = this->displayView(theView);
    if (didDisplay)
    {
      rsrc->properties().get<bool>()["smtk.attribute_panel.display_hint"] = true;
      // If the view was specified then set the advance level as well
      if (view)
      {
        m_attrUIMgr->setAdvanceLevel(advancedlevel);
      }
    }
  }
  this->updateTitle(theView);
  auto rsrcMgr = rsrc->manager();
  if (rsrcMgr)
  {
    std::weak_ptr<smtk::resource::Manager> weakResourceManager = rsrcMgr;
    QPointer<pqSMTKAttributePanel> self(this);
    m_observer = rsrcMgr->observers().insert(
      [this, weakResourceManager, self](
        const smtk::resource::Resource& attrRsrc, smtk::resource::EventType evnt) {
        // Does the panel still exist?
        if (self == nullptr)
        {
          return;
        }
        auto rsrc = m_rsrc.lock();
        if (
          rsrc == nullptr ||
          (evnt == smtk::resource::EventType::REMOVED && &attrRsrc == rsrc.get()))
        {
          // The application is removing the attribute resource we are viewing.
          // Clear out the panel and unobserve the manager.
          this->resetPanel(weakResourceManager.lock());
          this->updateTitle();
          m_seln = nullptr;
        }
      },
      std::numeric_limits<smtk::resource::Observers::Priority>::lowest(),
      /* initialize */ false,
      "pqSMTKAttributePanel: Clear panel if a removed resource is being displayed.");
  }
  m_rsrc = rsrc;

  return didDisplay;
}

void pqSMTKAttributePanel::updateTitle(const smtk::view::ConfigurationPtr& view)
{
  // By default the Panel's name is Attribute Editor
  std::string panelName = "Attribute Editor";
  if (view)
  {
    // Lets see if the view wants a different base name
    view->details().attribute("AttributePanelTitle", panelName);
    // Lets see if we are suppose to add the resource name to it
    if (view->details().attributeAsBool("IncludeResourceNameInPanel"))
    {
      auto rsrc = m_rsrc.lock();
      panelName = rsrc ? (panelName + '(' + rsrc->name() + ')') : panelName;
    }
  }
  this->setWindowTitle(panelName.c_str());
  Q_EMIT titleChanged(panelName.c_str());
}

bool pqSMTKAttributePanel::displayTaskAttribute(smtk::task::Task* task)
{
  using ResourceSet = std::
    set<smtk::attribute::Resource::WeakPtr, std::owner_less<smtk::attribute::Resource::WeakPtr>>;

  bool didDisplay = false;
  if (!task)
  {
    return didDisplay;
  }
  // Ask the \a task for relevant attribute resources which can provide a view
  // with the name specified by the task's style.
  smtk::common::TypeContainer taskConfigData;
  if (!task->getViewData(taskConfigData) || !taskConfigData.contains<ResourceSet>())
  {
    return didDisplay;
  }
  auto managers = task->manager()->managers();
  this->updateManagers(managers);
  auto& resourceSet(taskConfigData.get<ResourceSet>());
  auto styles = task->style();
  // Iterate over styles attached to the \a task:
  for (const auto& style : styles)
  {
    auto styleConfig = task->manager()->getStyle(style);
    // Does this style have a tag for us?
    if (styleConfig.contains("attribute-panel"))
    {
      auto panelConfig = styleConfig.at("attribute-panel");
      if (panelConfig.contains("attribute-editor"))
      {
        auto viewName = panelConfig.at("attribute-editor").get<std::string>();
        // std::cout << "Got view name " << viewName << std::endl;
        for (const auto& weakRsrc : resourceSet)
        {
          if (auto rsrc = std::dynamic_pointer_cast<smtk::attribute::Resource>(weakRsrc.lock()))
          {
            auto viewConfig = rsrc->findView(viewName);
            if (viewConfig)
            {
              // The task provided an attribute resource that
              // can provide view configuration given the view name;
              // we can proceed to display the view:
              this->resetPanel(rsrc->manager());
              // Replace the contents with UI for this view.
              didDisplay = this->displayResource(rsrc, viewConfig);
              if (didDisplay)
              {
                break;
              }
            }
          }
        }
        if (!didDisplay)
        {
          smtkWarningMacro(
            smtk::io::Logger::instance(),
            "Could not find an attribute view named \"" << viewName << "\".");
        }
        else
        {
          break;
        }
      }
    }
  }
  if (didDisplay)
  {
    // If we were able to show a view related to the task, start tracking
    // changes in the task state so we can mark the view read-only if the
    // task is completed (or read+write if marked incomplete).
    auto curState = task->state();
    m_currentTask = task;
    QPointer<pqSMTKAttributePanel> self = this;
    m_currentTaskObserverKey = task->observers().insert(
      [self](smtk::task::Task& task, smtk::task::State priorState, smtk::task::State currentState) {
        (void)task;
        (void)priorState;
        if (!self)
        {
          return;
        }
        self->m_attrUIMgr->setReadOnly(
          currentState < smtk::task::State::Incomplete ||
          currentState >= smtk::task::State::Completed);
      },
      "AttributePanel current task state-tracking.");
    this->m_attrUIMgr->setReadOnly(
      curState < smtk::task::State::Incomplete || curState >= smtk::task::State::Completed);
  }
  return didDisplay;
}

void pqSMTKAttributePanel::activeTaskStateChange(
  smtk::task::Task& task,
  smtk::task::State priorState,
  smtk::task::State currentState)
{
  if (currentState == smtk::task::State::Completed)
  {
    // Do not let the user edit this task's attribute any longer.
    if (auto resource = m_rsrc.lock())
    {
      this->resetPanel(resource->manager());
    }
  }
  else if (priorState == smtk::task::State::Completed)
  {
    this->displayTaskAttribute(&task);
  }
}
