//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_Agent_h
#define smtk_task_Agent_h

#include "smtk/CoreExports.h"
#include "smtk/SharedFromThis.h"
#include "smtk/SystemConfig.h"
#include "smtk/common/Deprecation.h"
#include "smtk/common/Managers.h"
#include "smtk/common/Observers.h"
#include "smtk/common/Visit.h"
#include "smtk/task/State.h"

#include "nlohmann/json.hpp"

#include <map>
#include <memory>
#include <string>
#include <utility>

namespace smtk
{
namespace task
{

class Port;
class PortData;
class Task;

///\brief Agent is a base class for all SMTK agents.
///
/// SMTK agents work within a Task and are used to both configure a task and
/// validate  task's state.  Agents can be assigned to the ports of a task and
/// control how the task responds to new data that is available on its input
/// ports as well as the data produced on its output ports.
///
/// Subclasses of Agent are responsible for providing custom behavior that is
/// required by the task in order for it to perform properly.

class SMTKCORE_EXPORT Agent
{
public:
  using State = smtk::task::State;
  using Configuration = nlohmann::json;
  smtkTypeMacroBase(smtk::task::Agent);

  Agent(Task* owningTask);

  virtual ~Agent() = default;

  ///\brief Return the current state of the agent
  virtual State state() const = 0;

  ///\brief Configure the agent based on a provided JSON configuration
  virtual void configure(const Configuration& config) = 0;

  ///\brief Return the agent's configuration.
  virtual Configuration configuration() const;

  ///\brief Return the port data from the agent.
  ///
  /// If the agent is not assigned to \a port, the method returns nullptr.
  virtual std::shared_ptr<PortData> portData(const Port* port) const;

  ///\brief  Tell the agent that the data on \a port has been updated.
  virtual void portDataUpdated(const Port* port);

  ///\brief Return the agent's parent task
  Task* parent() const { return m_parent; }

  ///\brief Return a description of actions users must take to
  ///       make the agent's state completable.
  ///
  /// Subclasses should override this method.
  ///
  /// This should be XHTML text that can be inserted into
  /// an unordered HTML list (<ul>…</ul>) along with the
  /// text from other agent instances.
  ///
  /// This should be an empty string when the agent's state is either
  /// completable or completed.
  virtual std::string troubleshoot() const { return std::string(); }

  ///\brief Insert view-related objects into \a configuration.
  ///
  /// Subclasses should override this; the default implementation does nothing.
  /// This method should return true when \a configuration is modified
  /// and false otherwise.
  virtual bool getViewData(smtk::common::TypeContainer& configuration) const;

protected:
  friend class Task; // So that tasks can notify their agents of state changes.

  /// Receive notification the parent Task's state has changed.
  virtual void taskStateChanged(State prev, State& next);

  ///\brief Receive notification that a Task's state has changed.
  ///
  /// This method would allow a Task to delegate children state
  /// calculation to an Agent instead
  virtual void taskStateChanged(Task* task, State prev, State next);

  // Agents must have a parent task in order to notify it
  // of state changes.
  smtk::task::Task* m_parent;
};

} // namespace task
} // namespace smtk

#endif // smtk_task_Task_h
