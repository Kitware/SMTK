//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_PortForwardingAgent_h
#define smtk_task_PortForwardingAgent_h

#include "smtk/task/Agent.h"

namespace smtk
{
namespace task
{

///\brief PortForwardingAgent verifies that attributes are valid.
///
class SMTKCORE_EXPORT PortForwardingAgent : public Agent
{
public:
  using State = smtk::task::State;
  using Configuration = nlohmann::json;

  /// A filter for objects
  struct ObjectFilter
  {
    /// A filter for types of resources to be forwarded.
    std::string m_resourceFilter;
    /// A filter for types of components to be forwarded.
    ///
    /// If this is empty, only resources matching m_resourceFilter will be forwarded.
    /// If this is "*", all components matching m_resourceFilter will be forwarded.
    std::string m_componentFilter;
  };

  /// Map from role name (or "*") to an array of object filters.
  using RolesToFilters = std::unordered_map<smtk::string::Token, std::vector<ObjectFilter>>;

  /// A rule for forwarding data.
  struct Forward
  {
    /// An input port to forward from.
    Port* m_inputPort{ nullptr };
    /// An output port to forward to.
    Port* m_outputPort{ nullptr };
    /// A map of roles whose matching objects should be forwarded.
    /// The domain of the map is role names and the range is a set
    /// of filters
    RolesToFilters m_filters;
    /// The role to which inputs should be forwarded on the output port.
    ///
    /// If invalid, the input roles will be used. Otherwise, all matching
    /// objects will appear on the given role.
    smtk::string::Token m_outputRole;
  };

  smtkSuperclassMacro(smtk::task::Agent);
  smtkTypeMacro(smtk::task::PortForwardingAgent);

  PortForwardingAgent(Task* owningTask);
  virtual ~PortForwardingAgent() = default;

  ///\brief Return the current state of the agent.
  ///
  /// This is Unavailable when no resources/attributes/definitions have
  /// been configured for validation; it is Incomplete when at least one
  /// attribute is configured for validation but is not in a valid state;
  /// and Completable when all the configured attributes are valid.
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
  ///
  /// This assumes that \a port is an input port of the task and
  /// forces portDataUpdated() to be called on each downstream
  /// of \a port.
  void portDataUpdated(const Port* port) override;

protected:
  std::vector<Forward> m_forwards;
};

} // namespace task
} // namespace smtk

#endif // smtk_task_Task_h
