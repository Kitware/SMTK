//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#include "smtk/job/Registrar.h"

#include "smtk/job/DatabaseQueue.h"
#include "smtk/job/Job.h"
#include "smtk/job/Queue.h"
#include "smtk/job/agents/JobAgent.h"
#include "smtk/job/operators/AddJobToQueue.h"
#include "smtk/job/operators/CancelJob.h"
#include "smtk/job/operators/JobUpdated.h"
#include "smtk/job/operators/ScheduleJob.h"

#include "smtk/attribute/ComponentItem.h"

#include "smtk/plugin/Manager.h"

#include <tuple>

using OperationList = std::tuple<
  smtk::job::AddJobToQueue,
  smtk::job::CancelJob,
  smtk::job::JobUpdated,
  smtk::job::ScheduleJob>;

using AgentList = std::tuple<smtk::job::agents::JobAgent>;

namespace smtk
{
namespace job
{
namespace
{

bool createdJobManager = false;

// An observer attached to the operation manager:
smtk::operation::Observers::Key g_operationObserver;

void attachJobToQueue(
  const smtk::operation::Operation&,
  smtk::operation::Operation::Result result,
  const smtk::operation::Manager::Ptr& operationManager)
{
  // Ignore jobs attached to failed operations.
  if (smtk::operation::outcome(result) != smtk::operation::Operation::Outcome::SUCCEEDED)
  {
    return;
  }
  auto jobsItem = result->findComponent("created");
  if (jobsItem && jobsItem->numberOfValues() > 0)
  {
    if (!operationManager)
    {
      smtkWarningMacro(smtk::io::Logger::instance(), "No operation manager. Cannot queue job(s).");
      return;
    }
    for (const auto& value : *jobsItem)
    {
      if (auto job = std::dynamic_pointer_cast<smtk::job::Job>(value))
      {
        // If a job was created but the queue does not know of it, then
        // launch an operation that will lock the queue in order to add
        // the job to the queue and potentially schedule it.
        //
        // This is done so that operations which create jobs need not
        // lock any queue which they plan to submit jobs to.
        if (job->queue())
        {
          auto adder = operationManager->create<smtk::job::AddJobToQueue>();
          adder->parameters()->associate(job);
          operationManager->launchers()(adder);
        }
      }
    }
  }
}

} // anonymous namespace

void Registrar::registerTo(const smtk::common::Managers::Ptr& managers)
{
  // Add a job::Manager if none is present:
  if (!managers->contains<smtk::job::Manager::Ptr>())
  {
    if (managers->insert(smtk::job::Manager::create()))
    {
      createdJobManager = true;
      // if (managers->contains<smtk::resource::Manager::Ptr>())
      // {
      //   managers->get<smtk::job::Manager::Ptr>()->registerResourceManager(
      //     managers->get<smtk::resource::Manager::Ptr>());
      // }
      // smtk::plugin::Manager::instance()->registerPluginsTo(
      //   managers->get<smtk::job::Manager::Ptr>());
    }
  }
}

void Registrar::unregisterFrom(const smtk::common::Managers::Ptr& managers)
{
  (void)managers;
  if (createdJobManager)
  {
    managers->erase<smtk::job::Manager::Ptr>();
  }
}

void Registrar::registerTo(const smtk::task::Manager::Ptr& taskManager)
{
  auto& agentFactory = taskManager->agentFactory();
  agentFactory.registerTypes<AgentList>();
}

void Registrar::unregisterFrom(const smtk::task::Manager::Ptr& taskManager)
{
  auto& agentFactory = taskManager->agentFactory();
  agentFactory.unregisterTypes<AgentList>();
}

void Registrar::registerTo(const smtk::job::Manager::Ptr&) {}

void Registrar::unregisterFrom(const smtk::job::Manager::Ptr&) {}

void Registrar::registerTo(const smtk::resource::Manager::Ptr& resourceManager)
{
  auto& typeLabels = resourceManager->objectTypeLabels();
  typeLabels[smtk::common::typeName<smtk::job::DatabaseQueue>()] = "database queue";
  typeLabels[smtk::common::typeName<smtk::job::Queue>()] = "queue";
  typeLabels[smtk::common::typeName<smtk::job::Job>()] = "job";
  smtk::string::Token dummy1("smtk::job::Queue");
  smtk::string::Token dummy2("smtk::job::DatabaseQueue");
  smtk::string::Token dummy3("smtk::job::Job");
}

void Registrar::unregisterFrom(const smtk::resource::Manager::Ptr&) {}

void Registrar::registerTo(const smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->registerOperations<OperationList>();

  g_operationObserver = operationManager->observers().insert(
    [&](
      const smtk::operation::Operation& op,
      smtk::operation::EventType event,
      smtk::operation::Operation::Result result) -> int {
      if (event == smtk::operation::EventType::DID_OPERATE)
      {
        attachJobToQueue(op, result, operationManager);
      }
      return 0; // Never cancel an operation.
    },
    /*priority*/ 0,
    /*initialize*/ false,
    "Observe operations for jobs to queue.");
}

void Registrar::unregisterFrom(const smtk::operation::Manager::Ptr&)
{
  g_operationObserver.release();
}
} // namespace job
} // namespace smtk
