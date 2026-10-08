//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// Verify that task activation applies ParaView mode and named view layouts,
// preserves existing layouts, and continues to work with multiple projects.
// Also cover legacy mode settings and JobAgent output-port persistence.

#include "smtk/common/Managers.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/job/pqArtifacts.h"
#include "smtk/extension/paraview/project/pqSMTKTaskResourceVisibility.h"
#include "smtk/job/Job.h"
#include "smtk/job/agents/JobAgent.h"
#include "smtk/project/Project.h"
#include "smtk/task/Manager.h"
#include "smtk/task/ObjectsInRoles.h"
#include "smtk/task/Port.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqMultiViewWidget.h"
#include "pqObjectBuilder.h"
#include "pqServer.h"
#include "pqServerManagerModel.h"
#include "pqServerResource.h"
#include "pqTabbedMultiViewWidget.h"
#include "pqView.h"
#include "pqViewFrame.h"

#include "vtkNew.h"
#include "vtkSMParaViewPipelineControllerWithRendering.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyManager.h"
#include "vtkSMSessionProxyManager.h"
#include "vtkSMViewLayoutProxy.h"
#include "vtkSMViewProxy.h"

#include <QApplication>
#include <QTextDocument>

#include <iostream>
#include <stdexcept>

namespace
{
// Expose protected style and project-event handlers so the test can exercise
// task activation with an explicitly selected task manager.
class TaskStyleBehavior : public pqSMTKTaskResourceVisibility
{
public:
  TaskStyleBehavior(smtk::task::Manager* manager)
    : pqSMTKTaskResourceVisibility(nullptr)
  {
    m_currentTaskManager = manager;
  }
  void useManager(smtk::task::Manager* manager) { m_currentTaskManager = manager; }
  using pqSMTKTaskResourceVisibility::applyJobResults;
  using pqSMTKTaskResourceVisibility::applyLayout;
  using pqSMTKTaskResourceVisibility::handleProjectEvent;
  using pqSMTKTaskResourceVisibility::processTaskEvent;
};

// Supply a job through an input port without constructing an entire workflow.
class JobInputTask : public smtk::task::Task
{
public:
  smtk::task::Port input;
  std::unordered_map<smtk::string::Token, smtk::task::Port*> inputPorts{ { "input", &input } };
  std::shared_ptr<smtk::task::ObjectsInRoles> data = std::make_shared<smtk::task::ObjectsInRoles>();
  const std::unordered_map<smtk::string::Token, smtk::task::Port*>& ports() const override
  {
    return inputPorts;
  }
  std::shared_ptr<smtk::task::PortData> portData(const smtk::task::Port*) const override
  {
    return data;
  }
};

// Allow a test job to be assigned directly when checking that a reloaded
// agent preserves its output-port configuration and publishes the job.
class TestJobAgent : public smtk::job::agents::JobAgent
{
public:
  using smtk::job::agents::JobAgent::JobAgent;
  void useJob(smtk::job::Job* job) { m_job = job; }
};

void check(bool condition, const char* message)
{
  if (!condition)
  {
    throw std::runtime_error(message);
  }
}
} // namespace

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);
  pqApplicationCore core(argc, argv);
  pqTabbedMultiViewWidget tabs;
  auto* server = core.getObjectBuilder()->createServer(pqServerResource("builtin:"));
  if (!server)
  {
    return 1;
  }
  pqActiveObjects::instance().setActiveServer(server);
  auto manager = smtk::task::Manager::create();
  const auto preStyle = nlohmann::json::parse(R"({
    "paraview-mode": false,
    "layout": { "name": "Pre-Processing", "views": [ { "type": "RenderView" } ] }
  })");
  const auto postStyle = nlohmann::json::parse(R"({
    "paraview-mode": true,
    "layout": {
      "name": "PostProcessing", "split": "vertical",
      "views": [
        { "type": "XYChartView", "name": "Flow Results" },
        { "split": "horizontal", "views": [
          { "type": "XYChartView", "name": "Probes" }, { "type": "XYChartView", "name": "Force Probes" }
        ] }
      ]
    }
  })");
  manager->setStyles({ { "default", postStyle } });
  TaskStyleBehavior behavior(manager.get());
  auto* mode = pqSMTKBehavior::instance();
  mode->setPostProcessingMode(false);
  int modeChanges = 0;
  QObject::connect(mode, &pqSMTKBehavior::postProcessingModeChanged, &app, [&modeChanges](bool) {
    ++modeChanges;
  });
  try
  {
    behavior.applyLayout("Original");
    auto* original = tabs.layoutProxy();
    check(original, "Could not create the original layout.");
    auto* pxm = server->proxyManager();
    const auto initialCount = pxm->GetNumberOfProxies("layouts");
    behavior.processTaskEvent(nullptr, "deactivated");
    check(!mode->postProcessingMode(), "Deactivation must not enable ParaView mode.");
    check(tabs.layoutProxy() == original, "Deactivation must not switch layouts.");

    behavior.processTaskEvent(nullptr, "activated");
    auto* post = vtkSMViewLayoutProxy::SafeDownCast(pxm->GetProxy("layouts", "PostProcessing"));
    check(mode->postProcessingMode(), "Activation must enable ParaView mode.");
    check(post && tabs.layoutProxy() == post, "Activation must create and select PostProcessing.");
    check(pxm->GetNumberOfProxies("layouts") == initialCount + 1, "Expected one new layout.");
    check(post->GetViews().size() == 3, "Expected exactly three chart views.");
    check(
      post->GetSplitDirection(0) == vtkSMViewLayoutProxy::VERTICAL, "Expected top/bottom split.");
    check(
      post->GetSplitDirection(2) == vtkSMViewLayoutProxy::HORIZONTAL,
      "Expected left/right split in the bottom half.");
    for (int location : { 1, 5, 6 })
    {
      auto* view = post->GetView(location);
      check(
        view && std::string(view->GetXMLName()) == "XYChartView",
        "Expected a Line Chart View at each leaf.");
    }
    auto checkNames = [&]() {
      const std::pair<int, const char*> names[] = { { 1, "Flow Results" },
                                                    { 5, "Probes" },
                                                    { 6, "Force Probes" } };
      for (const auto& entry : names)
      {
        const char* registeredName = pxm->GetProxyName("views", post->GetView(entry.first));
        check(
          registeredName && std::string(registeredName) == entry.second,
          "Chart must have its configured registered name.");
        auto* frame =
          tabs.findTab(post)->findChild<pqViewFrame*>(QString("Frame.%1").arg(entry.first));
        check(frame, "Expected a chart frame at the configured location.");
        QTextDocument title;
        title.setHtml(frame->title()); // The active frame uses bold/underlined HTML.
        check(title.toPlainText() == entry.second, "Chart frame must display its configured name.");
      }
    };
    checkNames();
    // Simulate a layout saved before view names were configured.
    auto* topView = core.getServerManagerModel()->findItem<pqView*>(post->GetView(1));
    topView->rename("Old unnamed chart");
    tabs.findTab(post)->findChild<pqViewFrame*>("Frame.1")->setTitle("Old unnamed chart");
    const auto chartViews = post->GetViews();
    post->SetSplitFraction(0, 0.4);

    behavior.applyLayout("Original");
    behavior.processTaskEvent(nullptr, "activated");
    check(tabs.layoutProxy() == post, "Reactivation must select the existing layout.");
    check(modeChanges == 1, "Already-enabled ParaView mode must not be toggled again.");
    check(pxm->GetNumberOfProxies("layouts") == initialCount + 1, "Must not duplicate layouts.");
    check(post->GetSplitFraction(0) == 0.4, "Reactivation must preserve layout splits.");

    check(post->GetViews() == chartViews, "Reactivation must preserve the chart views.");
    checkNames();

    manager->setStyles({ { "default", preStyle } });
    behavior.processTaskEvent(nullptr, "activated");
    auto* pre = tabs.layoutProxy();
    check(!mode->postProcessingMode(), "Setup tasks must turn ParaView mode off.");
    check(pre == pxm->GetProxy("layouts", "Pre-Processing"), "Expected Pre-Processing layout.");
    check(
      pre->GetViews().size() == 1 && std::string(pre->GetView(0)->GetXMLName()) == "RenderView",
      "Expected a single Render View.");

    // Layout selection must work without any postprocessing directive.
    manager->setStyles({ { "default", { { "layout", postStyle.at("layout") } } } });
    behavior.processTaskEvent(nullptr, "activated");
    check(
      tabs.layoutProxy() == post && !mode->postProcessingMode(),
      "Layout-only styles must not change ParaView mode.");
    check(post->GetViews() == chartViews, "Layout-only activation must reuse chart views.");

    behavior.applyLayout("Original");
    vtkNew<vtkSMParaViewPipelineControllerWithRendering> controller;
    for (auto* view : chartViews)
    {
      controller->UnRegisterProxy(view);
    }
    controller->UnRegisterProxy(post);
    // Populate the empty layout created by the original implementation.
    behavior.applyLayout("PostProcessing");
    check(tabs.layoutProxy()->GetViews().empty(), "Expected empty legacy layout.");
    manager->setStyles({ { "default", postStyle } });
    behavior.processTaskEvent(nullptr, "activated");
    check(
      tabs.layoutProxy() == pxm->GetProxy("layouts", "PostProcessing"),
      "Activation must recreate a deleted layout.");
    check(
      pxm->GetNumberOfProxies("layouts") == initialCount + 2,
      "Expected only the original, pre-processing, and postprocessing layouts.");
    check(tabs.layoutProxy()->GetViews().size() == 3, "Empty legacy layout must acquire charts.");

    const auto count = pxm->GetNumberOfProxies("layouts");
    behavior.applyLayout(nlohmann::json::parse(R"({
      "name": "Invalid", "views": [ { "type": "NoSuchView" } ]
    })"));
    check(
      pxm->GetNumberOfProxies("layouts") == count,
      "Invalid view templates must not leave partial layouts.");

    // Exercise actual task switches with a second project loaded, as happens
    // when a simulation loads its mesh project.
    auto simulation = smtk::project::Project::create();
    auto mesh = smtk::project::Project::create();
    auto& simulationTasks = simulation->taskManager();
    auto& meshTasks = mesh->taskManager();
    simulationTasks.setStyles(
      { { "default", preStyle }, { "inspect", postStyle }, { "choose", preStyle } });
    meshTasks.setStyles({ { "default", preStyle }, { "choose", preStyle } });
    simulationTasks.taskInstances().registerType<smtk::task::Task>();
    auto managers = smtk::common::Managers::create();
    auto inspect = simulationTasks.taskInstances().create<smtk::task::Task>(
      smtk::task::Task::Configuration{ { "name", "Inspect Results" }, { "style", { "inspect" } } },
      simulationTasks,
      managers);
    auto choose = simulationTasks.taskInstances().create<smtk::task::Task>(
      smtk::task::Task::Configuration{ { "name", "Choose Mesh" }, { "style", { "choose" } } },
      simulationTasks,
      managers);
    check(inspect && choose, "Could not create workflow tasks.");
    behavior.handleProjectEvent(*simulation, smtk::project::EventType::ADDED);
    QCoreApplication::processEvents();
    behavior.handleProjectEvent(*mesh, smtk::project::EventType::ADDED);
    QCoreApplication::processEvents();
    check(simulationTasks.active().switchTo(inspect.get()), "Could not activate Inspect Results.");
    check(
      mode->postProcessingMode() &&
        tabs.layoutProxy() == pxm->GetProxy("layouts", "PostProcessing"),
      "Inspect Results must still control mode and layout after loading a mesh project.");
    check(simulationTasks.active().switchTo(choose.get()), "Could not activate Choose Mesh.");
    check(
      !mode->postProcessingMode() && tabs.layoutProxy() == pre,
      "Choose Mesh must restore the setup mode and layout.");
    behavior.handleProjectEvent(*mesh, smtk::project::EventType::REMOVED);
    QCoreApplication::processEvents();
    check(
      simulationTasks.active().switchTo(inspect.get()), "Could not reactivate Inspect Results.");
    check(mode->postProcessingMode(), "Removing another project must preserve task observers.");
    behavior.handleProjectEvent(*simulation, smtk::project::EventType::REMOVED);
    QCoreApplication::processEvents();

    // JobAgent has a job-role but no output-role. Saving and reopening it must
    // still preserve the output port that supplies downstream Inspect Results.
    JobInputTask owner;
    TestJobAgent originalAgent(&owner);
    originalAgent.configure({ { "output-port", "output" },
                              { "job-role", "job" },
                              { "skip-update", true },
                              { "internal-state", "incomplete" } });
    const auto savedAgent = originalAgent.configuration();
    check(
      savedAgent.contains("output-port") && savedAgent.at("output-port") == "output",
      "Saving a JobAgent must preserve its output port even without output-role.");
    TestJobAgent reloadedAgent(&owner);
    auto reloadConfig = savedAgent;
    reloadConfig["skip-update"] = true;
    reloadedAgent.configure(reloadConfig);
    auto savedJob = smtk::job::Job::create();
    reloadedAgent.useJob(savedJob.get());
    smtk::task::Port output;
    output.setName("output");
    auto jobData =
      std::dynamic_pointer_cast<smtk::task::ObjectsInRoles>(reloadedAgent.portData(&output));
    check(
      jobData && jobData->data().at("job").count(savedJob.get()) == 1,
      "Reloaded JobAgent must provide its job on the output port.");

    // Saved workflows keep working; the new spelling takes precedence.
    manager->setStyles({ { "default", { { "postprocessing", { { "mode", true } } } } } });
    behavior.useManager(manager.get());
    behavior.processTaskEvent(nullptr, "activated");
    check(mode->postProcessingMode(), "Legacy mode directive must still work.");
    manager->setStyles(
      { { "default",
          { { "postprocessing", { { "mode", true } } }, { "paraview-mode", false } } } });
    behavior.processTaskEvent(nullptr, "activated");
    check(!mode->postProcessingMode(), "paraview-mode must override the legacy directive.");
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << '\n';
    return 1;
  }
  return 0;
}
