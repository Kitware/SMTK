//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_FillOutAttributes_h
#define smtk_task_FillOutAttributes_h

#include "smtk/operation/Manager.h"
#include "smtk/operation/Observer.h"
#include "smtk/operation/Operation.h"
#include "smtk/resource/Resource.h"
#include "smtk/task/AgentlessTask.h"

#include "smtk/common/Visit.h"

namespace smtk
{
namespace task
{
// Forward declaration
namespace adaptor
{
class ResourceAndRole;
}

/**\brief FillOutAttributes is a task that is incomplete until specified
  *       attributes are valid.
  *
  * This task accepts an input attribute resource (configured by a predecessor
  * task or specified via a role) and observe an operation manager for operations.
  * After each operation, attributes with a definition are validated.
  * If all attributes identify are valid, the task becomes completable.
  * Otherwise, the task will remain (or become) incomplete.
  *
  * FillOutAttributes has a single input and output port, both of which
  * pass PortData of type ObjectsInRoles.
  * The task itself is configured with AttributeSet instances that specify
  * a role and the port-data is searched for these roles in order to
  * configure attributes to be filled out.
  * The output port contains only the attribute(s) configured by this
  * task and does not include upstream port information so that when
  * the output port is marked as modified it is possible for downstream
  * tasks to be certain that it is the attributes in question that have
  * been updated.
  */
class SMTKCORE_EXPORT FillOutAttributes : public AgentlessTask
{
public:
  smtkTypeMacro(smtk::task::FillOutAttributes);
  smtkSuperclassMacro(smtk::task::AgentlessTask);
  smtkCreateMacro(smtk::resource::PersistentObject);

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
  };
  /// Signatures of functors that visit resources-by-role predicates.
  using AttributeSetVisitor = std::function<smtk::common::Visit(AttributeSet&)>;
  using ConstAttributeSetVisitor = std::function<smtk::common::Visit(const AttributeSet&)>;

  FillOutAttributes();
  FillOutAttributes(
    const Configuration& config,
    Manager& taskManager,
    const smtk::common::Managers::Ptr& managers = nullptr);
  FillOutAttributes(
    const Configuration& config,
    const PassedDependencies& dependencies,
    Manager& taskManager,
    const smtk::common::Managers::Ptr& managers = nullptr);

  ~FillOutAttributes() override = default;

  /// Parse configuration information to initialize this instance.
  void configure(const Configuration& config);

  /// Report this task's ports.
  const std::unordered_map<smtk::string::Token, Port*>& ports() const override;

  /// Return data for this task's output port.
  ///
  /// This task only has one port. Resources for each attribute-set's roles
  /// are reported. No upstream port data is transmitted to the downstream port.
  std::shared_ptr<PortData> portData(const Port* port) const override;

  /// Update the task based on new port data.
  void portDataUpdated(const Port* port) override;

  /// Provide the attribute resource(s) that the user should edit.
  bool getViewData(smtk::common::TypeContainer& configuration) const override;

  smtk::common::Visit visitAttributeSets(ConstAttributeSetVisitor visitor) const;
  smtk::common::Visit visitAttributeSets(AttributeSetVisitor visitor);

protected:
  friend class adaptor::ResourceAndRole;

  /// Initialize with a list of resources from manager in m_managers.
  bool initializeResources();
  /// Update a single resource in a predicate
  bool updateResourceEntry(
    smtk::attribute::Resource& resource,
    const AttributeSet& predicate,
    ResourceAttributes& entry);
  /// Respond to operations that may change task state.
  int update(
    const smtk::operation::Operation& op,
    smtk::operation::EventType event,
    smtk::operation::Operation::Result result);

  /// Check m_resourcesByRole to see if all requirements are met.
  State computeInternalState() const override;

  /// Determine if an attribute needs to be tested for its validity.  Returns true if the attribute was not already contained
  /// in resourceAtts
  static bool testValidity(
    const smtk::attribute::AttributePtr& attribute,
    ResourceAttributes& resourceAtts);

  SMTK_DEPRECATED_IN_24_11("Use hasRelevantInformation (proper spelling) instead.")
  bool hasRelevantInfomation(
    const smtk::resource::ManagerPtr& resourceManager,
    bool& foundResources) const
  {
    return this->hasRelevantInformation(resourceManager, foundResources);
  }

  /// Returns true if the task has relevant information in terms of its definitions and instances.
  ///
  /// The task has relevant information if any of its definitions' or instances' isRelevant() methods
  /// return true. It will also indicate if it found any of its required resources via the
  /// \a foundResources parameter.
  bool hasRelevantInformation(
    const smtk::resource::ManagerPtr& resourceManager,
    bool& foundResources) const;
  smtk::common::Managers::Ptr m_managers;
  smtk::operation::Observers::Key m_observer;
  std::vector<AttributeSet> m_attributeSets;
  std::unordered_map<smtk::string::Token, Port*> m_ports;
};
} // namespace task
} // namespace smtk

#endif // smtk_task_FillOutAttributes_h
