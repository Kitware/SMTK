//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_TrivialProducerAgent_h
#define smtk_task_TrivialProducerAgent_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/task/Agent.h"

namespace smtk
{
namespace task
{
class ObjectsInRoles;

///\brief TrivialProducerAgent outputs configured objects on output ports.
///
/// This agent exists so that operations such as EmplaceWorklet can
/// assign objects to a task. The downstream or child tasks of this
/// agent's task will then be configured with the objects in the roles
/// as configured.
///
/// Unless the task's configuration includes a minimum/maximum count
/// of objects per role, the task will always be completable.
class SMTKCORE_EXPORT TrivialProducerAgent : public Agent
{
public:
  using State = smtk::task::State;
  using Configuration = nlohmann::json;
  smtkSuperclassMacro(smtk::task::Agent);
  smtkTypeMacro(smtk::task::TrivialProducerAgent);

  TrivialProducerAgent(Task* owningTask);
  ~TrivialProducerAgent() override = default;

  ///\brief Return the current state of the agent.
  ///
  /// By default, this agent will always be completable, even if no resources are assigned.
  /// However, if the agent's configuration contains minimum/maximum counts
  /// for objects by role, the state will only be completable when the number of objects
  /// in each specified role is in the allowed range.
  State state() const override;

  ///\brief Configure the agent based on a provided JSON configuration.
  void configure(const Configuration& config) override;

  ///\brief Produce a JSON configuration object for the current task state.
  Configuration configuration() const override;

  ///\brief Provide feedback to users on how to make this agent completable.
  std::string troubleshoot() const override;

  ///\brief Return the port data from the agent.
  std::shared_ptr<PortData> portData(const Port* port) const override;

  ///\brief Tell the agent that the data on \a port has been updated.
  // void portDataUpdated(const Port* port) override;

  ///\brief Return the port this agent broadcasts its data to.
  Port* outputPort() const { return m_outputPort; }

  ///\brief A helper to add an object to a TrivialProducerAgent owned by a task.
  ///
  /// Note that if multiple agents match the same \a agentName or \a port,
  /// only the first occurence will have \a object inserted.
  ///
  /// We notify downstream observers of the agent's output port with a call
  /// to portDataUpdated(); be careful as this call to add objects is expected
  /// to be called during an operation (which may run on a non-GUI thread).
  static Port* addObjectInRole(
    Task* task,
    const std::string& agentName,
    smtk::string::Token role,
    smtk::resource::PersistentObject* object);
  static bool addObjectInRole(
    Task* task,
    Port* port,
    smtk::string::Token role,
    smtk::resource::PersistentObject* object);

  ///\brief A helper to remove an object from a TrivialProducerAgent owned by a task.
  ///
  /// Note that if multiple agents match the same \a agentName or \a port,
  /// only the first occurence will have \a object removed.
  ///
  /// We notify downstream observers of the agent's output port with a call
  /// to portDataUpdated(); be careful as this call to add objects is expected
  /// to be called during an operation (which may run on a non-GUI thread).
  static Port* removeObjectFromRole(
    Task* task,
    const std::string& agentName,
    smtk::string::Token role,
    smtk::resource::PersistentObject* object);
  static bool removeObjectFromRole(
    Task* task,
    Port* port,
    smtk::string::Token role,
    smtk::resource::PersistentObject* object);

  ///\brief A helper to reset a TrivialProducerAgent owned by a task.
  ///
  /// Note that if multiple agents match the same \a agentName, the
  /// first of them will be reset. If multiple agents match
  /// the same \a port, all of them will be reset.
  /// This is done so that the variant which accepts an \a agentName
  /// can return the modified Port. (Operations which modify ports must
  /// add them to the operation result.)
  ///
  /// We notify downstream observers of the agent's output port with a call
  /// to portDataUpdated(); be careful as this call to add objects is expected
  /// to be called during an operation (which may run on a non-GUI thread).
  static Port* resetData(Task* task, const std::string& agentName);
  static bool resetData(Task* task, Port* port);

protected:
  virtual State computeInternalState();

  State m_internalState{ State::Completable };
  std::shared_ptr<ObjectsInRoles> m_data;
  std::map<smtk::string::Token, std::pair<int, int>> m_requiredObjectCounts;
  Port* m_outputPort{ nullptr };
};

} // namespace task
} // namespace smtk

#endif // smtk_task_Task_h
