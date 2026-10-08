//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_JobRunnerAgent_h
#define smtk_task_JobRunnerAgent_h

#include "smtk/extension/qt/Exports.h" // For export macro.
#include "smtk/task/Agent.h"

#include <memory>

namespace smtk
{
namespace task
{

///\brief JobRunnerAgent ensures that a sequence of scripts has completed successfully
///       before its task is completable.
///
/// Each script may have a different set of input dependencies and output files.
/// Each script's output is placed in a separate log file.
///
/// This class depends on Qt for process and filesystem monitoring.
class SMTKQTEXT_EXPORT JobRunnerAgent : public Agent
{
public:
  using State = smtk::task::State;
  using Configuration = nlohmann::json;
  smtkSuperclassMacro(smtk::task::Agent);
  smtkTypeMacro(smtk::task::JobRunnerAgent);

  JobRunnerAgent(Task* owningTask);
  ~JobRunnerAgent() override;

  ///\brief Return the current state of the agent.
  ///
  /// This is Irrelevant when no job has been configured, Incomplete if either
  /// (1) no output files exist or (2) the runners have not been executed
  /// since last input-file timestamp; and Completable if both (1) output files
  /// are newer than input files and (2) runners were executed before last
  /// output-file timestamp.
  State state() const override;

  ///\brief Configure the agent based on a provided JSON configuration.
  void configure(const Configuration& config) override;

  ///\brief Return the agent's current configuration for serialization.
  Configuration configuration() const override;

  ///\brief Return the port data from the agent.
  ///
  /// If the agent is not assigned to \a port, the method returns nullptr.
  std::shared_ptr<PortData> portData(const Port* port) const override;

  ///\brief  Tell the agent that the data on \a port has been updated.
  void portDataUpdated(const Port* port) override;

protected:
  /// Receive notification the parent Task's state has changed.
  void taskStateChanged(State prev, State& next) override;

  ///\brief Receive notification that a child Task's state has changed.
  ///
  /// This method allows an agent to compute its parent Task's "child state"
  /// (assuming that duty falls upon this agent).
  void taskStateChanged(Task* task, State prev, State next) override;

private:
  class Internal;
  std::unique_ptr<Internal> m_p;
};

} // namespace task
} // namespace smtk

#endif // smtk_task_Task_h
