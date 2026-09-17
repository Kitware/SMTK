//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/job/agents/JobAgent.h"

#include "smtk/project/Manager.h"
#include "smtk/project/Project.h"

#include "smtk/job/Job.h"
#include "smtk/job/Manager.h"

#include "smtk/task/Manager.h"
#include "smtk/task/ObjectsInRoles.h"
#include "smtk/task/Task.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Resource.h"

#include "smtk/common/TypeContainer.h"

#include "smtk/string/json/jsonToken.h"

using namespace smtk::string::literals;

namespace smtk
{
namespace job
{
namespace agents
{

JobAgent::JobAgent(smtk::task::Task* owningTask)
  : SubmitOperationAgent(owningTask)
{
}

void JobAgent::configure(const Configuration& config)
{
  auto it = config.find("optional");
  m_optional = (it == config.end() ? false : it->get<bool>());
  it = config.find("rerun");
  m_rerun = (it == config.end() ? true : it->get<bool>());

  m_inputPort = nullptr;
  auto ipi = config.find("input-port");
  if (ipi != config.end())
  {
    auto ipname = ipi->get<smtk::string::Token>();
    auto ppi = m_parent->ports().find(ipname);
    m_inputPort = (ppi != m_parent->ports().end()) ? ppi->second : nullptr;
  }

  m_case.clear();
  it = config.find("case");
  if (it != config.end())
  {
    m_case = it->get<std::filesystem::path>();
  }

  m_jobTypeName.clear();
  it = config.find("job-type");
  if (it != config.end())
  {
    m_jobTypeName = it->get<std::string>();
  }

  it = config.find("job-role");
  if (it != config.end())
  {
    m_jobRole = it->get<std::string>();
  }
  else
  {
    m_jobRole.clear();
  }

  // Job outputs use job-role independently of SubmitOperationAgent's output-role.
  it = config.find("output-port");
  m_outputPortName = it == config.end() ? smtk::string::Token() : it->get<smtk::string::Token>();

  auto baseDir = this->caseDirectoryBase();
  it = config.find("job");
  if (it != config.end())
  {
    auto idit = it->find("id");
    auto quit = it->find("queue");
    if (idit == it->end() || quit == it->end())
    {
      smtkWarningMacro(smtk::io::Logger::instance(), "Job specified without a uuid or a queue id.");
    }
    else
    {
      if (auto mgrs = this->parent()->managers())
      {
        if (auto jobManager = mgrs->get<smtk::job::Manager::Ptr>())
        {
          if (auto queue = jobManager->queues().findById(quit->get<smtk::common::UUID>()))
          {
            m_job = queue->findJob(idit->get<smtk::common::UUID>()).get();
          }
        }
      }
    }
  }

  this->Superclass::configure(config);
  if (m_operation)
  {
    // Every time SubmitOperationAgent::configure is run, an operation is created.
    // So it is safe to always add a handler to the operation.
    // We use the handler to track the most recently created job.
    m_operation->addHandler(this->operationHandler(), /* priority */ 0);
  }

  // Initially, the EmplaceWorklet operation adds resource connections to
  // the parent task's internal output port, causing a cascade of
  // portDataUpdated() calls that complete the configuration by looking up
  // resources from the upstream ports.
  //
  // After the initial creation, all of this agent's ivars are deserialized properly.
}

JobAgent::Configuration JobAgent::configuration() const
{
  auto config = this->Superclass::configuration();
  config["rerun"] = m_rerun;
  if (m_optional)
  {
    config["optional"] = true;
  }
  if (m_inputPort)
  {
    config["input-port"] = m_inputPort->name();
  }
  if (!m_case.empty())
  {
    config["case"] = m_case;
  }
  if (!m_jobTypeName.empty())
  {
    config["job-type"] = m_jobTypeName;
  }
  if (!m_jobRole.empty())
  {
    config["job-role"] = m_jobRole;
  }
  if (m_outputPortName.valid())
  {
    config["output-port"] = m_outputPortName;
  }
  if (m_job && m_job->queue())
  {
    config["job"] = { { "id", m_job->id() }, { "queue", m_job->queue()->id() } };
  }
  return config;
}

std::shared_ptr<smtk::task::PortData> JobAgent::portData(const smtk::task::Port* port) const
{
  if (m_jobRole.empty() || !port || m_outputPortName != port->name() || !m_job)
  {
    return std::shared_ptr<smtk::task::PortData>();
  }
  auto data = std::make_shared<smtk::task::ObjectsInRoles>();
  data->addObject(m_job, m_jobRole);
  return data;
}

void JobAgent::portDataUpdated(const smtk::task::Port* port)
{
  auto prev = m_internalState;
  this->Superclass::portDataUpdated(port);
  // TODO: Update m_*FileName from attribute resource.
  if (m_internalState == prev)
  {
    // Our superclass didn't change state, but perhaps we should (if the
    // input port now provides caseDirectoryBase() with a value, for instance).
    m_parent->updateAgentState(this, prev, this->computeInternalState());
  }
}

bool JobAgent::getViewData(smtk::common::TypeContainer& configuration) const
{
  using ResourceSet = std::
    set<smtk::attribute::Resource::WeakPtr, std::owner_less<smtk::attribute::Resource::WeakPtr>>;

  auto resourceManager = m_parent->managers()->get<smtk::resource::Manager::Ptr>();
  if (!resourceManager)
  {
    return false;
  }

  // Fetch attribute resources on input port
  if (!m_inputPort)
  {
    return false;
  }
  auto* attRsrc = smtk::task::ObjectsInRoles::findTaskPortObjectInRoleAs<smtk::attribute::Resource>(
    m_parent, m_inputPort->name(), "problem setup"_token, "smtk::attribute::Resource"_token);
  if (!attRsrc)
  {
    return false;
  }

  ResourceSet viewData;
  if (configuration.contains<ResourceSet>())
  {
    viewData = configuration.get<ResourceSet>();
    for (const auto& weakResource : viewData)
    {
      if (auto resource = weakResource.lock())
      {
        if (resource.get() == attRsrc)
        {
          // The attribute resource was already present
          // (i.e., from a FillOutAttributesAgent).
          return false;
        }
      }
    }
  }

  viewData.insert(attRsrc->shared_from_this());
  configuration.insertOrAssign(viewData);
  return true;
}

std::filesystem::path JobAgent::caseDirectoryBase() const
{
  std::filesystem::path dir;
  if (!m_inputPort)
  {
    return dir;
  }

  auto pdata =
    std::dynamic_pointer_cast<smtk::task::ObjectsInRoles>(m_parent->portData(m_inputPort));
  if (!pdata)
  {
    return dir;
  }

  auto it = pdata->data().find("problem setup"_token);
  if (it == pdata->data().end() || it->second.empty())
  {
    return dir;
  }

  for (const auto& obj : it->second)
  {
    if (auto* rsrc = dynamic_cast<smtk::attribute::Resource*>(obj))
    {
      dir = rsrc->location();
      dir = dir.parent_path();
      return dir;
    }
  }
  return dir;
}

std::filesystem::path JobAgent::caseDirectory() const
{
  return this->caseDirectoryBase() / m_case;
}

smtk::job::Definition* JobAgent::jobType() const
{
  if (m_job)
  {
    return m_job->jobType();
  }

  if (!m_jobTypeName.empty())
  {
    if (auto jobManager = this->parent()->managers()->get<smtk::job::Manager::Ptr>())
    {
      return jobManager->jobTypes().findByName(m_jobTypeName).get();
    }
  }
  return nullptr;
}

JobAgent* JobAgent::jobAgent(const smtk::task::Task* task, smtk::string::Token agentName)
{
  if (!task)
  {
    return nullptr;
  }
  for (const auto& agent : task->agents())
  {
    if (auto* jobAgent = dynamic_cast<JobAgent*>(agent))
    {
      if (!agentName.valid() || jobAgent->name() == agentName)
      {
        return jobAgent;
      }
    }
  }
  return nullptr;
}

JobAgent* JobAgent::activeJobAgent(
  const smtk::common::TypeContainer& appContext,
  smtk::string::Token agentName)
{
  if (!appContext.contains<smtk::project::Manager::Ptr>())
  {
    return nullptr;
  }
  auto projMgr = appContext.get<smtk::project::Manager::Ptr>();
  if (!projMgr || projMgr->projects().empty())
  {
    return nullptr;
  }
  auto& taskMgr = (*projMgr->projects().begin())->taskManager();
  auto* activeTask = taskMgr.active().task();
  return JobAgent::jobAgent(activeTask, agentName);
}

void JobAgent::taskStateChanged(State prev, State& next)
{
  if (prev == smtk::task::State::Completed && next < prev)
  {
    // Require a re-run when the user marks this task as un-completed.
    m_rerun = true;
    // TODO: Perhaps also run all-clean? This would have the added benefit
    // of moving/removing the log file so that the state computation would match
    // without the need for m_rerun.
    if (!m_optional && next > smtk::task::State::Incomplete)
    {
      m_parent->updateAgentState(this, prev, smtk::task::State::Incomplete);
    }
  }
}

smtk::task::State JobAgent::computeInternalState()
{
  // If running a job is optional or we have no input file to track, the task is completable.
  if (m_optional)
  {
    m_internalState = smtk::task::State::Completable;
    return m_internalState;
  }
  if (
    !m_job || m_job->state() != smtk::job::State::Completed ||
    m_job->status() != smtk::job::Status::Succeeded)
  {
    m_internalState = smtk::task::State::Incomplete;
    return m_internalState;
  }
  m_internalState = smtk::task::State::Completable;
  return m_internalState;
#if 0
  // TODO: Track file dependencies based on mode and port data (e.g. is overset present)
  // If no log file exists, the task is incomplete.
  if (m_logFileName.empty() || !std::filesystem::exists(m_logFileName))
  {
    return smtk::task::State::Incomplete;
  }
  m_inputFileTime = std::filesystem::last_write_time(m_inputFileName);
  m_logFileTime = std::filesystem::last_write_time(m_logFileName);
  return m_inputFileTime < m_logFileTime ? smtk::task::State::Completable : smtk::task::State::Incomplete;
#endif
  return this->Superclass::computeInternalState();
}

bool JobAgent::setJob(smtk::job::Job* job)
{
  if (m_job == job)
  {
    return false;
  }
  if (m_job)
  {
    // We were watching a different job. Unobserve it.
    m_job->queue()->unobserve(m_job, m_jobObserver);
    m_jobObserver = 0;
  }
  m_job = job;
  if (m_job)
  {
    m_jobObserver = m_job->queue()->observe(m_job, this->jobHandler(), /* initialize */ true);
  }
  return true;
}

smtk::operation::Handler JobAgent::operationHandler()
{
  smtk::operation::Handler result = [this](
                                      smtk::operation::Operation& op,
                                      const std::shared_ptr<smtk::attribute::Attribute>& result) {
    // Always re-add ourselves as the operation completes:
    op.addHandler(this->operationHandler(), /*priority*/ 0);
    for (const auto& comp : *result->findComponent("created"))
    {
      if (auto job = std::dynamic_pointer_cast<smtk::job::Job>(comp))
      {
        this->setJob(job.get());
        break;
      }
    }
  };
  return result;
}

JobAgent::JobUpdateObserver JobAgent::jobHandler()
{
  JobUpdateObserver result = [this](const Job& job) {
    // Should we transition agent/task state?
    auto previous = this->Superclass::state();
    if (job.state() == Completed && job.status() == Succeeded)
    {
      // We have run a job successfully. If not already completable, we should become so.
      if (previous < smtk::task::State::Completable)
      {
        this->parent()->updateAgentState(this, previous, this->computeInternalState());
      }
    }
    else
    {
      // There is a new job but it has not been scheduled, completed, was canceled, or failed.
      // If completable, we should downgrade status to Incomplete.
      if (previous >= smtk::task::State::Completable)
      {
        this->parent()->updateAgentState(this, previous, this->computeInternalState());
      }
    }
  };
  return result;
}

} // namespace agents
} // namespace job
} // namespace smtk
