//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/project/pqSMTKTaskResourceVisibility.h"

#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKDiagramPanel.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"
#include "smtk/extension/paraview/project/Utility.h"
#include "smtk/extension/qt/diagram/qtDiagram.h"
#include "smtk/extension/qt/diagram/qtTaskEditor.h"
#include "smtk/extension/qt/diagram/qtTaskPath.h"

#include "smtk/job/Job.h"
#include "smtk/project/Manager.h"
#include "smtk/project/Project.h"

#include "smtk/task/Active.h"
#include "smtk/task/Manager.h"
#include "smtk/task/ObjectsInRoles.h"
#include "smtk/task/Port.h"
#include "smtk/task/Task.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqMultiViewWidget.h"
#include "pqPipelineSource.h"
#include "pqRepresentation.h"
#include "pqServer.h"
#include "pqServerManagerModel.h"
#include "pqTabbedMultiViewWidget.h"
#include "pqViewFrame.h"

#include "vtkNew.h"
#include "vtkSMParaViewPipelineControllerWithRendering.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMRepresentationProxy.h"
#include "vtkSMSessionProxyManager.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMViewLayoutProxy.h"
#include "vtkSMViewProxy.h"
#include "vtkSmartPointer.h"

#include <QRegularExpression>
#include <QTabWidget>
#include <QTimer>

#include <filesystem>
#include <functional>
#include <vector>

// Change "#undef" to "#define" to print messages as this behavior
// responds to changes in tasks with 3-d view style.
#undef SMTK_DBG_3D_STYLE

using namespace smtk::string::literals;

namespace
{

pqSMTKTaskResourceVisibility* gInstance = nullptr;

} // anonymous namespace

pqSMTKTaskResourceVisibility::pqSMTKTaskResourceVisibility(QObject* parent)
  : QObject(parent)
{
  auto* behavior = pqSMTKBehavior::instance();
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

  // Now update current state to take current server into account (if any).
  behavior->visitResourceManagersOnServers([this](pqSMTKWrapper* wrapper, pqServer* server) {
    this->observeProjectsOnServer(wrapper, server);
    return false; // terminate early
  });

  if (m_currentTaskManager && !m_currentTask)
  {
    this->processTaskEvent(nullptr, "activated");
  }

  auto* pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    pqCore->registerManager("smtk task resource visibility behavior", this);
  }
}

pqSMTKTaskResourceVisibility::~pqSMTKTaskResourceVisibility()
{
  if (this == gInstance)
  {
    gInstance = nullptr;
  }
}

pqSMTKTaskResourceVisibility* pqSMTKTaskResourceVisibility::instance(QObject* parent)
{
  if (!gInstance)
  {
    gInstance = new pqSMTKTaskResourceVisibility(parent);
  }
  return gInstance;
}

void pqSMTKTaskResourceVisibility::observeProjectsOnServer(pqSMTKWrapper* mgr, pqServer* server)
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

  QPointer<pqSMTKTaskResourceVisibility> self(this);
  auto observerKey = projectManager->observers().insert(
    [self](const smtk::project::Project& project, smtk::project::EventType event) {
      if (self)
      {
        self->handleProjectEvent(project, event);
      }
    },
    0,    // assign a neutral priority
    true, // immediatelyNotify
    "pqSMTKTaskResourceVisibility: Control resource visibility.");
  m_projectManagerObservers[projectManager] = std::move(observerKey);

  // m_taskWatcherKey = xx
}

void pqSMTKTaskResourceVisibility::unobserveProjectsOnServer(pqSMTKWrapper* mgr, pqServer* server)
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

void pqSMTKTaskResourceVisibility::handleProjectEvent(
  const smtk::project::Project& project,
  smtk::project::EventType event)
{
  auto* taskManager = const_cast<smtk::task::Manager*>(&project.taskManager());
  std::weak_ptr<smtk::task::Manager> weakTaskManager = taskManager->shared_from_this();
  switch (event)
  {
    case smtk::project::EventType::ADDED:
      // Reserve an entry now so removal can cancel deferred registration.
      if (!m_activeTaskObservers.emplace(taskManager, smtk::task::Active::Observers::Key()).second)
      {
        break;
      }
      // Wait for the operation adding the project to complete. Do not retain a
      // raw manager in the callback: the project may be removed before it runs.
      QTimer::singleShot(0, this, [this, weakTaskManager]() {
        auto manager = weakTaskManager.lock();
        if (!manager || m_activeTaskObservers.find(manager.get()) == m_activeTaskObservers.end())
        {
          return;
        }
        auto onActiveTask = [this,
                             weakTaskManager](smtk::task::Task* previous, smtk::task::Task* next) {
          auto manager = weakTaskManager.lock();
          if (!manager)
          {
            return;
          }
          if (m_currentTaskManager != manager.get())
          {
            // Apply the previous project's outgoing style using its own manager.
            if (m_currentTask)
            {
              this->processTaskEvent(m_currentTask, "deactivated"_token);
            }
            m_currentTask = nullptr;
            m_currentTaskManager = manager.get();
          }
          this->handleTaskEvent(previous, next);
        };
        auto& activeTracker = manager->active();
        m_activeTaskObservers.at(manager.get()) = activeTracker.observers().insert(
          onActiveTask,
          /* priority */ 0,
          /* initialize */ false,
          "Task tracking for visibility control.");
        // Loading a background project with no active task must not replace
        // the layout and mode of the project the user is already working on.
        if (activeTracker.task() || !m_currentTaskManager)
        {
          onActiveTask(activeTracker.task(), activeTracker.task());
        }
      });
      break;
    case smtk::project::EventType::REMOVED:
      // Erasing the key disconnects only this project's observer.
      m_activeTaskObservers.erase(taskManager);
      if (m_currentTaskManager == taskManager)
      {
        if (m_currentTask)
        {
          m_currentTask->observers().erase(m_currentTaskObserver);
        }
        m_currentTask = nullptr;
        m_currentTaskManager = nullptr;
      }
      break;
    case smtk::project::EventType::MODIFIED:
    default:
      // Do nothing.
      break;
  }
}

void pqSMTKTaskResourceVisibility::handleTaskEvent(
  smtk::task::Task* prevTask,
  smtk::task::Task* nextTask)
{
  // Stop observing the prior task (if any).
  m_currentTaskObserver.release();

  (void)prevTask;
  if (m_currentTask)
  {
    m_currentTask->observers().erase(m_currentTaskObserver);
    this->processTaskEvent(m_currentTask, "deactivated"_token);
    // self->displayResource(nullptr);
  }
  m_currentTask = nextTask;
  if (m_currentTask)
  {
    // self->displayTaskAttribute(nextTask);
  }
  else
  {
    // For tasks with children, if we deactivate a task without activating
    // any other, we should examine the diagram's task-path (breadcrumb) and
    // apply the trailing task's visual style so that resources for that task
    // are not hidden by "accident."
    auto* pqCore = pqApplicationCore::instance();
    if (pqCore)
    {
      if (auto* panel = dynamic_cast<pqSMTKDiagramPanel*>(pqCore->manager("smtk task panel")))
      {
        for (const auto& generatorEntry : panel->diagram()->generators())
        {
          if (
            const auto& taskEditor =
              std::dynamic_pointer_cast<smtk::extension::qtTaskEditor>(generatorEntry.second))
          {
            const auto* taskPath = taskEditor->taskPath();
            auto* parentTask = taskPath->lastTask();
            if (parentTask && parentTask->manager() == m_currentTaskManager)
            {
              m_currentTask = parentTask;
            }
          }
        }
      }
    }
  }
#ifdef SMTK_DBG_3D_STYLE
  std::cout << "Process \"activated\" event for \""
            << (m_currentTask ? m_currentTask->name() : "(default)") << "\".\n";
#endif
  this->processTaskEvent(m_currentTask, "activated"_token);
}

void pqSMTKTaskResourceVisibility::processTaskEvent(
  smtk::task::Task* task,
  smtk::string::Token event)
{
  std::unordered_set<smtk::string::Token> styleSet;
  if (task)
  {
    styleSet = task->style();
  }
  else
  {
    styleSet.insert("default"_token);
  }
  if (m_currentTaskManager)
  {
    for (const auto& styleTag : styleSet)
    {
      auto styleSpec = m_currentTaskManager->getStyle(styleTag);
      if (event == "activated"_token && styleSpec.contains("paraview-mode"))
      {
        const auto& mode = styleSpec.at("paraview-mode");
        if (mode.is_boolean())
        {
          pqSMTKBehavior::instance()->setPostProcessingMode(mode.get<bool>());
        }
        else
        {
          smtkWarningMacro(
            smtk::io::Logger::instance(),
            "ParaView mode for style " << styleTag.data() << " must be a boolean.");
        }
      }
      // Preserve saved workflows using the former nested directive. The new
      // spelling takes precedence if a style contains both forms.
      else if (event == "activated"_token && styleSpec.contains("postprocessing"))
      {
        auto ppSpec = styleSpec.at("postprocessing");
        auto it = ppSpec.find("mode");
        if (it == ppSpec.end())
        {
          smtkWarningMacro(
            smtk::io::Logger::instance(),
            "Postprocessing mode mentioned by style \"" << styleTag.data()
                                                        << "\" but has no mode.");
        }
        else
        {
          auto* behavior = pqSMTKBehavior::instance();
          behavior->setPostProcessingMode(it->get<bool>());
        }
      }
      if (event == "activated"_token && styleSpec.contains("layout"))
      {
        this->applyLayout(styleSpec.at("layout"));
      }
      if (styleSpec.contains("3d-view"))
      {
        auto viewSpec = styleSpec.at("3d-view");
        for (const auto& entry : viewSpec.items())
        {
          smtk::string::Token directive(entry.key());
          switch (directive.id())
          {
              // clang-format off
          case "color-by"_hash: this->applyColorBy(entry.value(), task, event); break;
          case "hide"_hash:     this->applyShowObjects(entry.value(), false, task, event); break;
          case "show"_hash:     this->applyShowObjects(entry.value(), true,  task, event); break;
          default:
            smtkWarningMacro(smtk::io::Logger::instance(),
              "Unknown directive \"" << directive.data()
              << "\" for style \"" << styleTag.data() << "\". Skipping.");
            break;
              // clang-format on
          }
        }
      }
      if (event == "activated"_token && task && styleSpec.contains("job-results"))
      {
        this->applyJobResults(styleSpec.at("job-results"), task);
      }
    }
  }
}

void pqSMTKTaskResourceVisibility::applyLayout(const nlohmann::json& spec)
{
  const auto nameSpec = spec.is_object() ? spec.value("name", nlohmann::json()) : spec;
  if (!nameSpec.is_string() || nameSpec.get<std::string>().empty())
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Layout must have a non-empty name.");
    return;
  }

  auto* core = pqApplicationCore::instance();
  auto* server = pqActiveObjects::instance().activeServer();
  auto* tabs =
    core ? qobject_cast<pqTabbedMultiViewWidget*>(core->manager("MULTIVIEW_WIDGET")) : nullptr;
  if (!server || !tabs)
  {
    smtkWarningMacro(
      smtk::io::Logger::instance(), "Cannot activate a layout without a server and view tabs.");
    return;
  }

  const auto name = nameSpec.get<std::string>();
  auto* pxm = server->proxyManager();
  vtkSmartPointer<vtkSMViewLayoutProxy> layout =
    vtkSMViewLayoutProxy::SafeDownCast(pxm->GetProxy("layouts", name.c_str()));
  // A view tree is a creation template. Preserve populated layouts and user edits.
  const bool populate = spec.is_object() && spec.contains("views") &&
    (!layout ||
     (layout->GetViews().empty() && layout->GetSplitDirection(0) == vtkSMViewLayoutProxy::NONE));
  struct Split
  {
    int location;
    int direction;
    double fraction;
  };
  struct View
  {
    int location;
    vtkSmartPointer<vtkSMViewProxy> proxy;
    std::string name;
  };
  std::vector<Split> splits;
  std::vector<View> views;
  vtkNew<vtkSMParaViewPipelineControllerWithRendering> controller;
  if (populate)
  {
    // Validate the whole tree and initialize its views before changing any layout.
    std::function<bool(const nlohmann::json&, int, int)> prepare;
    prepare = [&](const nlohmann::json& node, int location, int depth) {
      if (!node.is_object() || depth > 20)
      {
        return false;
      }
      if (node.contains("type"))
      {
        if (!node.at("type").is_string() || node.contains("views"))
        {
          return false;
        }
        const auto viewName = node.value("name", nlohmann::json(""));
        if (!viewName.is_string() || (node.contains("name") && viewName.get<std::string>().empty()))
        {
          return false;
        }
        const auto type = node.at("type").get<std::string>();
        if (!vtkSMViewProxy::SafeDownCast(pxm->GetPrototypeProxy("views", type.c_str())))
        {
          return false;
        }
        vtkSmartPointer<vtkSMViewProxy> view;
        view.TakeReference(vtkSMViewProxy::SafeDownCast(pxm->NewProxy("views", type.c_str())));
        if (!view || !controller->InitializeProxy(view))
        {
          return false;
        }
        views.push_back({ location, view, viewName.get<std::string>() });
        return true;
      }
      auto children = node.find("views");
      if (children == node.end() || !children->is_array())
      {
        return false;
      }
      if (children->size() == 1)
      {
        return prepare(children->at(0), location, depth + 1);
      }
      if (children->size() != 2 || !node.contains("split") || !node.at("split").is_string())
      {
        return false;
      }
      const auto direction = node.at("split").get<std::string>();
      const auto fractionSpec = node.value("fraction", nlohmann::json(0.5));
      if ((direction != "horizontal" && direction != "vertical") || !fractionSpec.is_number())
      {
        return false;
      }
      const auto fraction = fractionSpec.get<double>();
      if (!(fraction > 0.0 && fraction < 1.0))
      {
        return false;
      }
      splits.push_back({ location,
                         direction == "vertical" ? vtkSMViewLayoutProxy::VERTICAL
                                                 : vtkSMViewLayoutProxy::HORIZONTAL,
                         fraction });
      return prepare(children->at(0), 2 * location + 1, depth + 1) &&
        prepare(children->at(1), 2 * location + 2, depth + 1);
    };
    if (!prepare(spec, 0, 0))
    {
      smtkErrorMacro(smtk::io::Logger::instance(), "Invalid view tree for layout " << name << ".");
      return;
    }
  }
  if (!layout)
  {
    layout.TakeReference(vtkSMViewLayoutProxy::SafeDownCast(pxm->NewProxy("misc", "ViewLayout")));
    if (
      !layout || !controller->InitializeProxy(layout) ||
      !controller->RegisterLayoutProxy(layout, name.c_str()))
    {
      smtkErrorMacro(smtk::io::Logger::instance(), "Could not create layout " << name << ".");
      return;
    }
  }

  for (const auto& split : splits)
  {
    layout->Split(split.location, split.direction, split.fraction);
  }
  for (const auto& view : views)
  {
    controller->RegisterViewProxy(view.proxy, view.name.empty() ? nullptr : view.name.c_str());
    layout->AssignView(view.location, view.proxy);
    // New render views need representations of resources already loaded in the
    // application. Task visibility/color directives are applied after the layout.
    if (view.proxy->IsA("vtkSMRenderViewProxy"))
    {
      auto* model = core->getServerManagerModel();
      auto* pqview = model->findItem<pqView*>(view.proxy);
      for (auto* resource : model->findItems<pqSMTKResource*>(server))
      {
        pqSMTKBehavior::instance()->createRepresentation(resource, pqview);
      }
    }
  }

  // Apply explicit names to matching existing views as well as newly created ones.
  // Renaming a proxy does not refresh ParaView's frame title, so update both.
  auto* layoutTab = tabs->findTab(layout);
  std::function<void(const nlohmann::json&, int, int)> nameViews;
  nameViews = [&](const nlohmann::json& node, int location, int depth) {
    if (!node.is_object() || depth > 20)
    {
      return;
    }
    if (node.contains("type"))
    {
      auto* view = layout->GetView(location);
      auto name = node.find("name");
      if (
        view && node.at("type") == view->GetXMLName() && name != node.end() && name->is_string() &&
        !name->get<std::string>().empty())
      {
        const auto title = QString::fromStdString(name->get<std::string>());
        if (auto* pqview = core->getServerManagerModel()->findItem<pqView*>(view))
        {
          if (pqview->getSMName() != title)
          {
            pqview->rename(title);
          }
        }
        if (layoutTab)
        {
          for (auto* frame : layoutTab->findChildren<pqViewFrame*>())
          {
            if (frame->property("FRAME_INDEX").toInt() == location)
            {
              frame->setTitle(title);
            }
          }
        }
      }
      return;
    }
    auto children = node.find("views");
    if (children != node.end() && children->is_array())
    {
      if (children->size() == 1)
      {
        nameViews(children->at(0), location, depth + 1);
      }
      else if (children->size() == 2)
      {
        nameViews(children->at(0), 2 * location + 1, depth + 1);
        nameViews(children->at(1), 2 * location + 2, depth + 1);
      }
    }
  };
  nameViews(spec, 0, 0);

  // Registration creates the tab. Select it explicitly so empty layouts also work,
  // without creating another tab or changing views in an existing layout.
  if (auto* tab = tabs->findTab(layout))
  {
    for (auto* tabWidget : tabs->findChildren<QTabWidget*>())
    {
      const int index = tabWidget->indexOf(tab);
      if (index >= 0)
      {
        tabs->setCurrentTab(index);
        tab->makeFrameActive();
        return;
      }
    }
  }
}

void pqSMTKTaskResourceVisibility::applyJobResults(
  const nlohmann::json& spec,
  smtk::task::Task* task)
{
  if (
    !task || !spec.is_object() || !spec.contains("reader") || !spec.at("reader").is_string() ||
    !spec.contains("routes") || !spec.at("routes").is_object())
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Job results require a reader and port-to-view routes.");
    return;
  }
  auto* core = pqApplicationCore::instance();
  auto* server = pqActiveObjects::instance().activeServer();
  auto* tabs =
    core ? qobject_cast<pqTabbedMultiViewWidget*>(core->manager("MULTIVIEW_WIDGET")) : nullptr;
  auto* layout = tabs ? tabs->layoutProxy() : nullptr;
  if (!server || !layout)
  {
    return;
  }
  auto* pxm = server->proxyManager();
  auto* model = core->getServerManagerModel();
  vtkNew<vtkSMParaViewPipelineControllerWithRendering> controller;
  const auto layoutViews = layout->GetViews();
  // Hide previously displayed job results, including when the current job has
  // failed or has no results. Leave manually plotted sources untouched.
  for (auto* source : model->findItems<pqPipelineSource*>(server))
  {
    auto* proxy = source->getSourceProxy();
    if (proxy->GetAnnotation("smtk.job-results"))
    {
      for (auto* view : layoutViews)
      {
        for (unsigned int port = 0; port < proxy->GetNumberOfOutputPorts(); ++port)
        {
          controller->Hide(proxy, port, view);
        }
        if (auto* pqview = model->findItem<pqView*>(view))
        {
          QTimer::singleShot(0, pqview, [pqview]() { pqview->render(); });
        }
      }
    }
  }
  auto* job = smtk::task::ObjectsInRoles::findTaskPortObjectInRoleAs<smtk::job::Job>(
    task,
    spec.value("port", std::string("input")),
    spec.value("role", std::string("job")),
    "smtk::job::Job");
  if (
    !job || job->state() != smtk::job::State::Completed ||
    job->status() != smtk::job::Status::Succeeded || job->caseDirectory().empty())
  {
    return;
  }
  // The job is authoritative: its case directory may differ from the study's
  // conventional foam/sim location (e.g. relocated or customized runs).
  std::error_code error;
  auto directory = job->caseDirectory() / spec.value("directory", std::string("postProcessing"));
  if (!std::filesystem::is_directory(directory, error))
  {
    return;
  }
  directory = std::filesystem::canonical(directory, error);
  if (error)
  {
    return;
  }
  const auto path = directory.string();
  const auto readerType = spec.at("reader").get<std::string>();
  const auto jobId = job->id().toString();
  vtkSmartPointer<vtkSMSourceProxy> reader;
  for (auto* source : model->findItems<pqPipelineSource*>(server))
  {
    auto* proxy = source->getSourceProxy();
    const char* id = proxy->GetAnnotation("smtk.job-results");
    if (
      id && jobId == id && readerType == proxy->GetXMLName() && proxy->GetProperty("FileName") &&
      path == vtkSMPropertyHelper(proxy, "FileName").GetAsString())
    {
      reader = proxy;
      break;
    }
  }
  if (!reader)
  {
    reader.TakeReference(
      vtkSMSourceProxy::SafeDownCast(pxm->NewProxy("sources", readerType.c_str())));
    if (!reader || !reader->GetProperty("FileName"))
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(), "Cannot create job-results reader " << readerType);
      return;
    }
    controller->PreInitializeProxy(reader);
    vtkSMPropertyHelper(reader, "FileName").Set(path.c_str());
    controller->PostInitializeProxy(reader);
    reader->UpdateVTKObjects();
    reader->UpdatePipeline();
    reader->SetAnnotation("smtk.job-results", jobId.c_str());
    controller->RegisterPipelineProxy(reader, "Simulation Results");
    if (auto* source = model->findItem<pqPipelineSource*>(reader))
    {
      source->setModifiedState(pqProxy::UNMODIFIED);
    }
  }
  reader->UpdatePipeline();
  for (unsigned int port = 0; port < reader->GetNumberOfOutputPorts(); ++port)
  {
    const auto portName = QString::fromUtf8(reader->GetOutputPortName(port));
    for (const auto& route : spec.at("routes").items())
    {
      if (!route.value().is_string())
      {
        continue;
      }
      const QRegularExpression pattern(
        QRegularExpression::wildcardToRegularExpression(QString::fromStdString(route.key())));
      if (!pattern.match(portName).hasMatch())
      {
        continue;
      }
      const auto target = route.value().get<std::string>();
      for (auto* view : layoutViews)
      {
        const char* name = pxm->GetProxyName("views", view);
        if (!name || target != name)
        {
          continue;
        }
        if (auto* representation = controller->Show(reader, port, view))
        {
          if (representation->GetProperty("UseIndexForXAxis"))
          {
            vtkSMPropertyHelper(representation, "UseIndexForXAxis").Set(0);
            vtkSMPropertyHelper(representation, "XArrayName").Set("Time");
            vtkSMPropertyHelper(representation, "SeriesVisibility").SetStatus("Time", 0);
            representation->UpdateVTKObjects();
          }
          if (auto* pqview = model->findItem<pqView*>(view))
          {
            QTimer::singleShot(0, pqview, [pqview]() { pqview->render(); });
          }
        }
      }
    }
  }
}

void pqSMTKTaskResourceVisibility::applyColorBy(
  const nlohmann::json& spec,
  smtk::task::Task* task,
  smtk::string::Token event)
{
  (void)task;
  (void)event;

  if (!spec.is_object() || !spec.contains("mode"))
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Specification is not a dictionary or has no mode:\n"
        << spec.dump(2) << "\n");
    return;
  }
  auto representations =
    smtk::paraview::relevantRepresentations(spec, m_currentTaskManager, m_currentTask);
  auto mode(spec.at("mode").get<smtk::string::Token>());
  bool needsRender = false;
  for (const auto& entry : representations)
  {
    auto* representation = entry.first;
    // auto* proxy = vtkSMTKResourceRepresentation::SafeDownCast(representation.getProxy());
    auto* proxy = representation->getProxy();
    if (!proxy)
    {
      continue;
    }

    switch (mode.id())
    {
      case "none"_hash:
        vtkSMPropertyHelper(proxy, "ColorBy").Set("None");
        needsRender = true;
        break;
      default:
      case "attribute-association"_hash:
        smtkErrorMacro(smtk::io::Logger::instance(), "Unsupported color mode.");
        break;
      case "entity"_hash:
        vtkSMPropertyHelper(proxy, "ColorBy").Set("Entity");
        needsRender = true;
        break;
    }
  }
  if (needsRender)
  {
    if (auto* view = pqActiveObjects::instance().activeView())
    {
      view->render();
    }
  }
}

void pqSMTKTaskResourceVisibility::applyShowObjects(
  const nlohmann::json& specArray,
  bool show,
  smtk::task::Task* task,
  smtk::string::Token event)
{
  (void)specArray;
  (void)show;
  (void)task;
  (void)event;

  if (!specArray.is_array())
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Specification is not an array:\n"
        << specArray.dump(2) << "\n");
    return;
  }
  bool needsRender = false;
  for (const auto& spec : specArray)
  {
    if (!spec.is_object())
    {
      smtkErrorMacro(
        smtk::io::Logger::instance(),
        "Specification is not a dictionary or has no mode:\n"
          << spec.dump(2) << "\n");
      return;
    }
    auto representations =
      smtk::paraview::relevantRepresentations(spec, m_currentTaskManager, m_currentTask);
    for (const auto& entry : representations)
    {
      auto* representation = entry.first;
      // auto* proxy = vtkSMTKResourceRepresentation::SafeDownCast(representation.getProxy());

      if (representation->isVisible() != show)
      {
        representation->setVisible(show);
        needsRender = true;
      }
    }
  }
  if (needsRender)
  {
    if (auto* view = pqActiveObjects::instance().activeView())
    {
      view->render();
    }
  }
}
