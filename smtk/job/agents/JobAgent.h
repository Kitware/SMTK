//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_job_agents_JobAgent_h
#define smtk_job_agents_JobAgent_h

#include "smtk/CoreExports.h"
#include "smtk/task/SubmitOperationAgent.h"

#include <filesystem>
#include <functional>

namespace smtk
{
namespace job
{

class Definition;
class Job;
class Queue;

namespace agents
{

/**\brief JobAgent verifies that a job created by an operation has run successfully.
  *
  * This agent also accepts data from an input port; the first resource provided
  * in a "problem setup" role will have its containing directory advertised as the
  * "caseDirectoryBase" path. Thus jobs related to resources may be kept in the same
  * directory as the resource itself.
  *
  * This agent inherits SubmitOperationAgent; the operation it is expected to
  * submit is one which creates a job.
  * If the operation provides a ComponentItem named "task," this agent's parent
  * task will be set and the operation can query this agent for a directory to use
  * for its case files.
  *
  * If the agent is configured as optional, it will always be completable
  * but will still store information about the last job run.
  *
  * This agent stores a pointer to the most recently submitted job (via its operation),
  * if any exists.
  *
  * Typically, this agent will be used with a pqJobRunnerView so that users can
  * monitor the progress of the job.
  */
class SMTKCORE_EXPORT JobAgent : public smtk::task::SubmitOperationAgent
{
public:
  using State = smtk::task::State;
  using Configuration = smtk::task::Agent::Configuration;
  using JobUpdateObserver = std::function<void(const Job& job)>;
  smtkSuperclassMacro(smtk::task::SubmitOperationAgent);
  smtkTypeMacro(smtk::job::agents::JobAgent);

  JobAgent(smtk::task::Task* owningTask);
  ~JobAgent() override = default;

  ///\brief Configure the agent based on a provided JSON configuration.
  void configure(const Configuration& config) override;

  ///\brief Return the agent's current configuration for serialization.
  Configuration configuration() const override;

  ///\brief Return the port data from the agent.
  ///
  /// If the agent is not assigned to \a port, the method returns nullptr.
  std::shared_ptr<smtk::task::PortData> portData(const smtk::task::Port* port) const override;

  ///\brief  Tell the agent that the data on \a port has been updated.
  void portDataUpdated(const smtk::task::Port* port) override;

  ///\brief Provide views with data required.
  ///
  /// This adds the attribute resource from the agent's input to a ResourceSet
  /// so that the attribute panel can locate view configurations.
  bool getViewData(smtk::common::TypeContainer& configuration) const override;

  /// Path (relative to an attribute resource location) of case directories.
  ///
  /// This is a convenience method to search input port data for an attribute
  /// resource and return its location. Operations may then ask this agent
  /// for a base directory into which they place the job's case directory.
  ///
  /// A single attribute resource may require multiple jobs run (i.e., one to
  /// produce a mesh, one to perform simulation). The caseDirectoryBase should
  /// have subdirectories if so.
  std::filesystem::path caseDirectoryBase() const;

  /// Report the case directory that jobs created by this agent will use.
  ///
  /// By default, this returns caseDirectoryBase(), but if the configuration
  /// JSON provides "case" as a relative path, that will be appended to
  /// the caseDirectoryBase().
  std::filesystem::path caseDirectory() const;

  /// Return the job from the most recent run of this agent's operation
  /// (or null if none has been run).
  smtk::job::Job* job() const { return m_job; }

  /// Return the name of the job definition that jobs for this agent will use.
  ///
  /// This is not used by the agent but may be used by user interfaces
  /// to prepare for jobs even before a job has been created.
  ///
  /// If the agent may return jobs of different types, the returned value will
  /// be empty.
  ///
  /// The default is an empty job type name but if the configuration JSON
  /// contains a "job-type" value, it will be returned.
  std::string jobTypeName() const { return m_jobTypeName; }

  /// Return the job definition (type) that jobs for this agent will use.
  ///
  /// This may be null if the agent may create jobs of different types and
  /// no job has been created. Once a job has been created, this method will
  /// return this->job()->jobType().
  smtk::job::Definition* jobType() const;

  /// Return the first job agent of \a task with the given \a agentName
  /// (or the first agent of this type if \a agentName is invalid).
  ///
  /// This is a convenience.
  static JobAgent* jobAgent(const smtk::task::Task* task, smtk::string::Token agentName);

  /// Fetch the active task and call JobAgent::jobAgent(agentName).
  static JobAgent* activeJobAgent(
    const smtk::common::TypeContainer& appContext,
    smtk::string::Token agentName);

  smtk::task::Port* inputPort() const { return m_inputPort; }

protected:
  /// Receive notification the parent Task's state has changed.
  void taskStateChanged(State prev, State& next) override;

  ///\brief Compute the current state of the agent.
  ///
  /// If m_optional is true (such as for tasks like refining meshes, where
  /// users may choose to submit a job or not), this state will always
  /// be completable.
  ///
  /// Otherwise, the state is incomplete when no job has completed successfully.
  /// If the task ever transitions from State::Completed to a lower State,
  /// the timestamp will be updated so that a re-rerun is required (unless
  /// m_optional is true).
  ///
  /// The state computation also uses the SubmitOperationAgent's state
  /// to determine its own state.
  State computeInternalState() override;

  /// Set the most recent job created by this agent's operation.
  bool setJob(smtk::job::Job* job);

  /// Return an operation handler to be added to this agent's operation.
  ///
  /// The handler updates the agent's most recent job.
  smtk::operation::Handler operationHandler();

  /// Return a job handler to be invoked when a job changes stage/state/status.
  ///
  /// The handler may update the state of this agent's parent task.
  JobUpdateObserver jobHandler();

  /// Is running the job optional or mandatory?
  bool m_optional{ false };

  /// If m_optional is false, this is true when we require a re-run.
  bool m_rerun{ true };

  /// The port from which to draw input resources used to specify the job.
  ///
  /// For now, this agent uses fixed roles on this port.
  smtk::task::Port* m_inputPort{ nullptr };

  /// The job created by this agent's operation.
  smtk::job::Job* m_job{ nullptr };

  /// The key of our "job observer" watching m_job (or 0).
  int m_jobObserver{ 0 };

  /// The relative path from the caseDirectoryBase() to the
  /// case directory for this agent's jobs.
  std::filesystem::path m_case;

  /// The job type for jobs this agent produces.
  ///
  /// This is not used by the agent but made available to user interface
  /// elements that may need to pre-populate with information about jobs.
  std::string m_jobTypeName;

  /// If set, this is the role on the output port (configured as
  /// part of SubmitOperationAgent) in which the job will appear.
  std::string m_jobRole;
};

} // namespace agents
} // namespace job
} // namespace smtk

#endif // smtk_job_agents_JobAgent_h
