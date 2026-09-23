//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// Verify that restored job agents use the current job status rather than stale
// saved state, preserve task completion, and reconnect job-update observers.
#include "smtk/job/agents/JobAgent.h"
#include "smtk/common/Managers.h"
#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/job/Manager.h"
#include "smtk/job/Registrar.h"
#include "smtk/task/Manager.h"
#include "smtk/task/Task.h"

namespace
{
// Provide persisted-job lookup and deterministic status notifications without
// launching a process or accessing the user's job database.
class RestoredJobQueue : public smtk::job::Queue
{
public:
  std::shared_ptr<smtk::job::Job> job = smtk::job::Job::create();
  RestoredJobQueue() { job->setQueue(this); }
  std::shared_ptr<smtk::job::Job> findJob(const smtk::common::UUID& id) const override
  {
    return id == job->id() ? job : nullptr;
  }
  void notify()
  {
    for (const auto& entry : m_observers[job->id()])
    {
      entry.second(*job);
    }
  }
  std::size_t observerCount() const
  {
    auto it = m_observers.find(job->id());
    return it == m_observers.end() ? 0 : it->second.size();
  }
};
} // namespace

int jobAgent(int, char*[])
{
  using smtk::task::State;
  for (bool completed : { false, true })
  {
    auto managers = smtk::common::Managers::create();
    auto jobs = smtk::job::Manager::create();
    managers->insert(jobs);
    auto tasks = smtk::task::Manager::create();
    tasks->setManagers(managers);
    smtk::job::Registrar::registerTo(tasks);
    auto queue = std::make_shared<RestoredJobQueue>();
    jobs->queues().manage(queue);
    queue->job->setState(smtk::job::State::Completed);
    queue->job->setStatus(smtk::job::Status::Succeeded);
    smtk::task::Task::Configuration agentConfig = {
      { "type", "smtk::job::agents::JobAgent" },
      { "internal-state", "incomplete" },
      { "skip-update", true },
      { "job", { { "id", queue->job->id() }, { "queue", queue->id() } } }
    };
    auto task = std::make_shared<smtk::task::Task>(
      smtk::task::Task::Configuration{ { "name", "Restored simulation" },
                                       { "completed", completed },
                                       { "agentState", "incomplete" },
                                       { "agents", { agentConfig } } },
      *tasks,
      managers);
    test(task->agents().size() == 1, "Expected a restored job agent.");
    auto* agent = dynamic_cast<smtk::job::agents::JobAgent*>(*task->agents().begin());
    test(
      agent && agent->state() == State::Completable,
      "A successful restored job must override stale incomplete agent state.");
    test(
      task->state() == (completed ? State::Completed : State::Completable),
      "Restoration must preserve the saved task completion flag.");
    test(queue->observerCount() == 1, "Restoration must observe the job.");

    // Reconfiguration with the same job must also reconcile stale state without
    // accumulating observers or temporarily clearing the task completion flag.
    agent->configure(agentConfig);
    test(queue->observerCount() == 1, "Reconfiguration must not duplicate observers.");
    test(
      task->state() == (completed ? State::Completed : State::Completable),
      "Reconfiguration must preserve task completion.");
    queue->job->setStatus(smtk::job::Status::Failed);
    queue->notify();
    test(task->state() == State::Incomplete, "Restored observers must report job failure.");
    queue->job->setStatus(smtk::job::Status::Succeeded);
    queue->notify();
    test(task->state() == State::Completable, "Restored observers must report job success.");

    // A missing job cannot validate success, even if the saved agent was completable.
    agentConfig.erase("job");
    agentConfig["internal-state"] = "completable";
    agent->configure(agentConfig);
    test(agent->state() == State::Incomplete, "Missing jobs must not retain stale success.");
    test(queue->observerCount() == 0, "Replacing a job must detach its observer.");
  }
  return 0;
}
