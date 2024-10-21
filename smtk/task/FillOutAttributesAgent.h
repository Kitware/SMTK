//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_FillOutAttributesAgent_h
#define smtk_task_FillOutAttributesAgent_h

#include "smtk/PublicPointerDefs.h"

#include "smtk/operation/Observer.h"
#include "smtk/operation/Operation.h"
#include "smtk/task/Agent.h"

namespace smtk
{
namespace task
{

/**\brief FillOutAttributesAgent verifies that attributes are valid.
  *
  * This agent accepts an input attribute resource (configured by hand
  * or by PortData of type ObjectsInRoles) and observes an operation
  * manager to determine when relevant attributes have been modified.
  * Once all attributes identify are valid, the agent becomes completable.
  * Otherwise, the task will remain (or become) incomplete.
  * The agent is unavailable if there are no resources or attributes
  * to validate.
  *
  * FillOutAttributesAgent accepts a single input and output port,
  * both of which pass PortData of type ObjectsInRoles.
  * The agent itself is configured with AttributeSet instances that
  * specify a role and the port-data is searched for these roles in
  * order to configure attributes to be filled out.
  *
  * The output port contains only the attribute(s) configured by this
  * task and does not include upstream port information so that when
  * the output port is marked as modified it is possible for downstream
  * tasks to be certain that it is the attributes in question that have
  * been updated.
  */
class SMTKCORE_EXPORT FillOutAttributesAgent : public Agent
{
public:
  using State = smtk::task::State;
  using Configuration = nlohmann::json;
  smtkSuperclassMacro(smtk::task::Agent);
  smtkTypeMacro(smtk::task::FillOutAttributesAgent);

  FillOutAttributesAgent(Task* parent);
  ~FillOutAttributesAgent() override = default;

  ///\brief Return the current state of the agent.
  ///
  /// This is Unavailable when no resources/attributes/definitions have
  /// been configured for validation; it is Incomplete when at least one
  /// attribute is configured for validation but is not in a valid state;
  /// and Completable when all the configured attributes are valid.
  State state() const override;

  ///\brief Configure the agent based on a provided JSON configuration.
  void configure(const Configuration& config) override;

  ///\brief Return the agent's current configuration.
  Configuration configuration() const override;

  ///\brief Return the port data from the agent.
  ///
  /// If the agent is not assigned to \a port, the method returns nullptr.
  std::shared_ptr<PortData> portData(const Port* port) const override;

  ///\brief  Tell the agent that the data on \a port has been updated.
  void portDataUpdated(const Port* port) override;

  /// Per-resource sets of validated attributes
  ///
  /// We need to track attributes so incremental updates
  /// can decide whether to change state.
  struct ResourceAttributes
  {
    /// Attributes matching a definition that are validated.
    std::set<smtk::common::UUID> m_valid;
    /// Attributes matching a definition that need attention.
    std::set<smtk::common::UUID> m_invalid;
  };

  /// Which objects should be broadcast as output port-data.
  enum class PortDataObjects
  {
    Resources,  //!< Each resource containing attributes to be validated should be added.
    Attributes, //!< Each attribute to be validated should be added.
    Both        //!< Both resources and attributes should be added as port data.
  };

  /// A predicate used to collect resources that fit a given role.
  struct AttributeSet
  {
    /// The required role. If empty, any role is allowed.
    std::string m_role;
    /// The definitions in matching resources whose attributes should be valid.
    std::set<std::string> m_definitions;
    /// The explicit attribute instances in matching resources that need to be valid.
    std::set<std::string> m_instances;
    /// Should all resources with a matching role be added?
    ///
    /// If false (default), then resources must be explicitly configured by UUID
    /// or configured by a task adaptor.
    /// If true, then all resources with a matching role will have attributes
    /// matching m_definitions checked.
    bool m_autoconfigure = false;
    /// The set of resources being managed that are selected by the validator.
    std::map<smtk::common::UUID, ResourceAttributes> m_resources;
    /// The objects to place in output port-data.
    PortDataObjects m_outputData{ PortDataObjects::Resources };
  };

  /// Convert to/from a PortDataObjects enumerant.
  static smtk::string::Token PortDataObjectsToken(PortDataObjects value);
  static PortDataObjects PortDataObjectsValue(smtk::string::Token token);

  /// Provide read-only access to the agent's configuration.
  const std::vector<AttributeSet>& attributeSets() const { return m_attributeSets; }

  /// The port (if any) that the agent accepts configuration data from.
  Port* inputPort() const { return m_inputPort; }

  /// The port (if any) that the agent pushes its configured attribute resources.
  Port* outputPort() const { return m_outputPort; }

  /// Return the set of attribute-resource UUIDs relevant to this agent.
  bool getViewData(smtk::common::TypeContainer& configuration) const;

protected:
  /// Receive notification the parent Task's state has changed.
  void taskStateChanged(State prev, State& next) override;

  ///\brief Receive notification that a Task's state has changed.
  ///
  /// This method would allow a Task to delegate children state
  /// calculation to an FillOutAttributesAgent instead
  void taskStateChanged(Task* task, State prev, State next) override;

  /// Initialize with a list of resources from manager in m_managers.
  bool initializeResources();

  /// Update a single resource in a predicate.
  bool updateResourceEntry(
    smtk::attribute::Resource& resource,
    const AttributeSet& predicate,
    ResourceAttributes& entry);

  /// Respond to operations that may change task state.
  int update(
    const smtk::operation::Operation& op,
    smtk::operation::EventType event,
    smtk::operation::Operation::Result result);

  /// Return true if the agent is configured with resources to validate.
  ///
  /// Resources are only counted as relevant if they contain definition names
  /// or attribute instance-names that match the agent's configuration.
  bool hasRelevantInformation(
    const smtk::resource::ManagerPtr& resourceManager,
    bool& foundResources) const;

  /// Check m_resourcesByRole to see if all requirements are met.
  virtual State computeInternalState();

  /// Determine if an attribute needs to be tested for its validity.
  ///
  /// Returns true if the attribute was not already contained in \a resourceAtts.
  static bool testValidity(
    const smtk::attribute::AttributePtr& attribute,
    ResourceAttributes& resourceAtts);

  /// Current agent state.
  /// This is set by computeInternalState and to be used when calling m_parent->updateTaskState().
  State m_internalState{ State::Unavailable };
  /// Observe operations for changes to relevant attributes.
  smtk::operation::Observers::Key m_observer;
  /// The list of resources/attributes that are relevant to this agent's state.
  std::vector<AttributeSet> m_attributeSets;
  /// Points to the input port (if runtime configuration is allowed); may be null.
  Port* m_inputPort{ nullptr };
  /// Points to the output port (if configured to provide configuration data).
  /// If null, updates to the configuration are not propagated downstream.
  Port* m_outputPort{ nullptr };
};

} // namespace task
} // namespace smtk

#endif // smtk_task_Task_h
