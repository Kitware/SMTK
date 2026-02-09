//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_qt_agents_JobRunnerAgent_h
#define smtk_qt_agents_JobRunnerAgent_h

#include "smtk/extension/qt/agents/JobRunnerAgent.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/operation/JobSpecs.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Observer.h"
#include "smtk/operation/Operation.h"
#include "smtk/task/Task.h"

#include <QProcess>

#include "nlohmann/json.hpp"

namespace smtk
{
namespace task
{

class JobRunnerAgent::Internal
{
public:
  Internal(JobRunnerAgent* self)
    : m_self(self)
  {
    if (self && self->parent())
    {
      if (auto context = m_self->parent()->managers())
      {
        if (auto opMgr = context->get<smtk::operation::Manager::Ptr>())
        {
          m_operationObserver = opMgr->observers().insert(
            [&](
              const smtk::operation::Operation& op,
              smtk::operation::EventType event,
              smtk::operation::Operation::Result result) -> int {
              if (event == smtk::operation::EventType::DID_OPERATE)
              {
                this->possiblyLaunchJob(op, result);
              }
              return 0; // Never cancel an operation.
            },
            /*priority*/ 0,
            /*initialize*/ false,
            "Observe operations for job-runner.");
        }
      }
    }
  }

  void possiblyLaunchJob(
    const smtk::operation::Operation& op,
    smtk::operation::Operation::Result result)
  {
    if (smtk::operation::outcome(result) != smtk::operation::Operation::Outcome::SUCCEEDED)
    {
      return;
    }
    if (op.typeName() != m_configuration["monitor-operation"].get<std::string>())
    {
      return;
    }

#if 0
    auto jobTaskItem = result->findComponent("job-task");
    smtk::task::Task::Ptr jobTask = jobTaskItem && jobTaskItem->isSet() ?
      jobTaskItem->valueAs<smtk::task::Task>() : smtk::task::Task::Ptr();
    if (jobTask.get() != m_self->parent())
    {
      return;
    }
    // Finally we know we should launch a job. Now see if the \a result specifies
    // one and launch it.
    auto scripts = result->findGroup("job-scripts");
    if (scripts) { return; }
#endif

    auto& scriptConfig = m_configuration["scripts"];
    scriptConfig = nlohmann::json::array();
    smtk::operation::visitJobSpecs(
      result,
      "JobSpec",
      [&](
        const smtk::attribute::ReferenceItem::Ptr& tasks,
        const std::filesystem::path& caseDirectory,
        smtk::string::Token jobLocation,
        smtk::string::Token jobQueueing,
        smtk::string::Token jobLauncher,
        const std::vector<std::pair<std::filesystem::path, std::vector<std::filesystem::path>>>&
          scriptLogs) {
        // No task:
        if (!tasks || !tasks->isSet())
        {
          return;
        }
        // Task doesn't match parent task:
        if (tasks->value().get() != m_self->parent())
        {
          return;
        }

        m_configuration["case-directory"] = caseDirectory;
        m_configuration["job-location"] = jobLocation.data();
        m_configuration["job-queue-system"] = jobQueueing.data();
        m_configuration["job-launcher"] = jobLauncher.data();
        for (auto [scriptPath, logPaths] : scriptLogs)
        {
          scriptConfig.emplace_back<nlohmann::json>(
            { { "script", scriptPath }, { "logs", logPaths } });
        }
        std::cout << "Case " << caseDirectory << " scripts (" << jobLocation.data() << ", "
                  << jobQueueing.data() << ", " << jobLauncher.data() << " (" << scriptLogs.size()
                  << "))"
                  << "start job"
                  << "\n";
      });
  }

  void updateConfiguration(const nlohmann::json& config)
  {
    // TODO: Actually perform update.
    // TODO: If there are active processes, either (1) error out as we cannot
    //       reconfigure while there are jobs running or (2) kill running jobs and
    //       start new ones as required. The simpler option is 1.
    m_configuration = config;
  }

  JobRunnerAgent* m_self{ nullptr };
  QProcess* m_process{ nullptr };
  nlohmann::json m_configuration;
  bool m_lastRunOK{ false };
  smtk::operation::Observers::Key m_operationObserver;
};

JobRunnerAgent::JobRunnerAgent(Task* owningTask)
  : smtk::task::Agent(owningTask)
  , m_p(std::make_unique<Internal>(this))
{
}

JobRunnerAgent::~JobRunnerAgent()
{
  // TODO: terminate or join all subprocesses, clean up log analyzers.
}

State JobRunnerAgent::state() const
{
  return m_p->m_lastRunOK ? smtk::task::State::Completable : smtk::task::State::Incomplete;
}

void JobRunnerAgent::configure(const Configuration& config)
{
  m_p->updateConfiguration(config);
}

JobRunnerAgent::Configuration JobRunnerAgent::configuration() const
{
  return m_p->m_configuration;
}

std::shared_ptr<PortData> JobRunnerAgent::portData(const Port* port) const
{
  return std::shared_ptr<PortData>();
}

void JobRunnerAgent::portDataUpdated(const Port* port)
{
  (void)port;
  // TODO: gather resources from input port.
}

void JobRunnerAgent::taskStateChanged(State prev, State& next)
{
  (void)prev;
  (void)next;
  // TODO
}

void JobRunnerAgent::taskStateChanged(Task* task, State prev, State next)
{
  (void)task;
  (void)prev;
  (void)next;
  // TODO
}

} // namespace task
} // namespace smtk
#endif // smtk_qt_agents_JobRunnerAgent_h
