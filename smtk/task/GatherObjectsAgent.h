//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_GatherObjectsAgent_h
#define smtk_task_GatherObjectsAgent_h

#include "smtk/task/Agent.h"

#include "smtk/task/ObjectsInRoles.h"

#include "smtk/common/UUID.h"

#include "nlohmann/json.hpp"

namespace smtk
{
namespace task
{

///\brief GatherObjectsAgent broadcasts programmatically-provided data on an output port.
///
class SMTKCORE_EXPORT GatherObjectsAgent : public Agent
{
public:
  using State = smtk::task::State;
  using Configuration = nlohmann::json;
  using DataMap = std::unordered_map<
    smtk::string::Token,
    std::unordered_map<smtk::common::UUID, std::unordered_set<smtk::common::UUID>>>;
  smtkSuperclassMacro(smtk::task::Agent);
  smtkTypeMacro(smtk::task::GatherObjectsAgent);

  GatherObjectsAgent(Task* owningTask);
  ~GatherObjectsAgent() override = default;

  ///\brief Return the current state of the agent.
  ///
  /// This agent is always completable.
  State state() const override;

  ///\brief Configure the agent based on a provided JSON configuration.
  void configure(const Configuration& config) override;

  ///\brief Return JSON configuration for this agent's current state.
  Configuration configuration() const override;

  ///\brief Return the port data from the agent.
  ///
  /// If the agent is not assigned to \a port, the method returns nullptr.
  std::shared_ptr<PortData> portData(const Port* port) const override;

  ///\brief Add a persistent \a object to this port's output in the given \a role.
  ///
  /// This returns true if the object was added.
  /// If the port was modified and \a signal is true,
  /// then Task::portDataUpdated() is invoked for this agent's port.
  bool addObjectInRole(
    smtk::resource::PersistentObject* object,
    smtk::string::Token role,
    bool signal = false);

  ///\brief Remove a persistent \a object from this port's output for the given \a role.
  ///
  /// This returns true if the object was removed.
  /// If the port was modified and \a signal is true,
  /// then Task::portDataUpdated() is invoked for this agent's port.
  bool removeObjectFromRole(
    smtk::resource::PersistentObject* object,
    smtk::string::Token role,
    bool signal = false);

  ///\brief Clear the data to be reported on the output port.
  ///
  /// If the port was modified and \a signal is true,
  /// then Task::portDataUpdated() is invoked for this agent's port.
  bool clearOutputPort(bool signal = false);

  /// Return this agent's output port (or null).
  Port* outputPort() const;

  /// Return this agent's configuration data.
  const DataMap& data() const { return m_objects; }

protected:
  /// The name of the parent-task's port this agent should provided data for.
  smtk::string::Token m_outputPortName;

  /// The data to present on the specified output port.
  ///
  /// This is kept in the form of UUIDs so we can avoid the need to
  /// monitor operations and clean the map as operations expunge objects.
  /// If an object is missing, it will not be reported in the port-data.
  DataMap m_objects;
};

} // namespace task
} // namespace smtk

#endif // smtk_task_Task_h
