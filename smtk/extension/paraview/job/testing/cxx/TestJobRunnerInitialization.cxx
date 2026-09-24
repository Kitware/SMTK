//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
/**\brief Guard against a crash when constructing a job view for a reopened project.
 *
 * Allowing Artifact controls to inspect partial results introduced a call from
 * updateJobStage() to pqJobRunnerView::currentJob(). The initial stage update ran
 * inside Internal's constructor, before std::make_unique<Internal>() returned and
 * assigned the view's m_p. Reopening a project with an existing job therefore
 * dereferenced a null m_p while accessing the cached job's weak_ptr.
 *
 * This test constructs the real Qt view with a job already attached to its active
 * task. It checks initialization for successful, failed, and running jobs, plus
 * access to a failed job's partial artifact after a reader marker is created.
 * The fixture models the state after project restoration; it does not deserialize
 * a project, submit a job, run a solver, or load artifact geometry. It refreshes
 * the view explicitly, so it does not test filesystem-watcher event delivery or
 * application-specific debug controls.
 */
#include "smtk/attribute/Resource.h"
#include "smtk/common/Managers.h"
#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/extension/paraview/job/pqJobRunnerView.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/job/Definition.h"
#include "smtk/job/Job.h"
#include "smtk/job/Stage.h"
#include "smtk/job/agents/JobAgent.h"
#include "smtk/operation/Manager.h"
#include "smtk/project/Manager.h"
#include "smtk/project/Project.h"
#include "smtk/resource/Manager.h"
#include "smtk/task/Manager.h"
#include "smtk/task/Task.h"
#include "smtk/view/Configuration.h"
#include "smtk/view/Information.h"

#include <QApplication>
#include <QFile>
#include <QPushButton>
#include <QTemporaryDir>

namespace
{
// Supply the post-restoration job directly, without requiring a queue or its
// persistence machinery. The caller keeps the job alive while views use it.
class RestoredJobAgent : public smtk::job::agents::JobAgent
{
public:
  RestoredJobAgent(smtk::task::Task* task, smtk::job::Job* job)
    : JobAgent(task)
  {
    m_job = job;
  }
};

// Attach the agent to a real task so the view discovers it through the same
// active-project/task lookup used when opening an existing workflow.
class RestoredTask : public smtk::task::Task
{
public:
  RestoredTask(
    smtk::task::Manager& manager,
    const smtk::common::Managers::Ptr& managers,
    smtk::job::Job* job)
    : Task(Configuration{ { "name", "Restored block mesh" } }, manager, managers)
  {
    m_agents.insert(std::make_unique<RestoredJobAgent>(this, job));
  }
};
} // namespace

int main(int argc, char** argv)
{
  QApplication app(argc, argv);
  auto resources = smtk::resource::Manager::create();
  auto operations = smtk::operation::Manager::create();
  auto projects = smtk::project::Manager::create(resources, operations);
  projects->registerProject<smtk::project::Project>();
  // JobRunner locates its agent through the project manager, not by accepting
  // the test job directly. Register and activate a task before creating the view.
  auto managers = smtk::common::Managers::create();
  managers->insert(resources);
  managers->insert(operations);
  managers->insert(projects);
  auto project = projects->create<smtk::project::Project>(managers);
  test(project && projects->add(project), "Create a managed project.");
  auto& tasks = project->taskManager();
  auto type = smtk::job::Definition::create();
  // Declaring an artifact is essential: a stage without artifacts would skip
  // creation of the controls whose initialization originally triggered the crash.
  type->appendStage("mesh", "Generate mesh")->addArtifact("mesh.marker");
  auto job = smtk::job::Job::create();
  job->setJobType(type);
  QTemporaryDir directory;
  test(directory.isValid(), "Create a temporary case directory.");
  job->setCaseDirectory(directory.path().toStdString());
  auto task = std::make_shared<RestoredTask>(tasks, managers, job.get());
  tasks.taskInstances().manage(task);
  test(tasks.active().switchTo(task.get()), "Activate the restored task.");

  auto attributes = smtk::attribute::Resource::create();
  smtk::extension::qtUIManager ui(attributes);
  ui.managers().insert(projects);
  auto config = smtk::view::Configuration::New("JobRunner", "Restored job");
  QWidget parent;
  smtk::view::Information info;
  info.insert<QWidget*>(&parent);
  info.insert<smtk::extension::qtUIManager*>(&ui);
  info.insert(config);

  // Construct the actual view with a pre-existing job: this formerly called
  // currentJob() from Internal's constructor before the view's m_p was assigned.
  for (auto status : { smtk::job::Succeeded, smtk::job::Failed, smtk::job::Pending })
  {
    // Success has passed the only stage. Failure and a running job remain at
    // stage zero, where no artifact exists yet. Each iteration creates a fresh
    // view to exercise construction rather than just updates to an existing view.
    job->setStatus(status);
    job->setState(status == smtk::job::Pending ? smtk::job::Running : smtk::job::Completed);
    job->setStage(status == smtk::job::Succeeded ? 1 : 0);
    pqJobRunnerView view(info);
    test(view.currentJob() == job.get(), "Construction must expose the restored job.");
    QPushButton* artifacts = nullptr;
    for (auto* button : view.widget()->findChildren<QPushButton*>())
    {
      if (button->toolTip() == "Artifact controls")
      {
        artifacts = button;
      }
    }
    test(artifacts != nullptr, "The restored stage must have artifact controls.");
    // Preserve stage-based availability for success; failed and running jobs
    // must not gain access merely because they have an artifact declaration.
    test(
      artifacts->isEnabled() == (status == smtk::job::Succeeded),
      "Initialize artifact availability from the restored job.");
    if (status == smtk::job::Failed)
    {
      // Model the Debug action's filesystem effect without changing job status
      // or stage. An explicit refresh must expose the stopped job's partial data.
      QFile marker(directory.filePath("mesh.marker"));
      test(marker.open(QIODevice::WriteOnly), "Create a partial result marker.");
      marker.close();
      view.updateUI();
      test(artifacts->isEnabled(), "A failed job's partial artifact must remain inspectable.");
      // Keep the next initialization scenario independent of this marker.
      marker.remove();
    }
  }
  return 0;
}
