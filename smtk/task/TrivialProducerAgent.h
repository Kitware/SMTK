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
class SMTKCORE_EXPORT TrivialProducerAgent : public Agent
{
public:
  using State = smtk::task::State;
  using Configuration = nlohmann::json;
  smtkSuperclassMacro(smtk::task::Agent);
  smtkTypeMacro(smtk::task::TrivialProducerAgent);

  TrivialProducerAgent(Task* owningTask);
  virtual ~TrivialProducerAgent() = default;

  ///\brief Return the current state of the agent.
  ///
  /// This agent will always be completable, even if no resources are assigned.
  State state() const override;

  ///\brief Configure the agent based on a provided JSON configuration
  void configure(const Configuration& config) override;

  ///\brief Return the port data from the agent.
  std::shared_ptr<PortData> portData(const Port* port) const override;

  ///\brief Tell the agent that the data on \a port has been updated.
  // void portDataUpdated(const Port* port) override;

  ///\brief Return a name for this agent, which may be empty.
  const std::string& name() const { return m_name; }

  ///\brief Return the port this agent broadcasts its data to.
  Port* outputPort() const { return m_outputPort; }

  ///\brief A helper to add an object to a TrivialProducerAgent owned by a task.
  ///
  /// Note that if multiple agents match the same \a agentName or \a port,
  /// only the first occurence will have \a object inserted.
  static bool addObjectInRole(
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
  static bool removeObjectFromRole(
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
  /// Note that if multiple agents match the same \a agentName or \a port,
  /// all of them will be reset.
  static bool resetData(Task* task, const std::string& agentName);
  static bool resetData(Task* task, Port* port);

protected:
  std::string m_name;
  std::shared_ptr<ObjectsInRoles> m_data;
  Port* m_outputPort{ nullptr };
};

} // namespace task
} // namespace smtk

#endif // smtk_task_Task_h
