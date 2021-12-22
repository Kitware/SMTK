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
#include "smtk/task/Task.h"

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

/**\brief Creating a Task that requires an operation to be executed
 * NOTE - this is just a place holder - as you can see this file and
 * the corresponding Cxx file are copies of FillOutAttribute.
  */
class SMTKCORE_EXPORT FillOutAttributes : public Task
{
public:
  smtkTypeMacro(smtk::task::FillOutAttributes);
  smtkSuperclassMacro(smtk::task::Task);
  smtkCreateMacro(smtk::task::Task);

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

  FillOutAttributes();
  FillOutAttributes(
    const Configuration& config,
    const smtk::common::Managers::Ptr& managers = nullptr);
  FillOutAttributes(
    const Configuration& config,
    const PassedDependencies& dependencies,
    const smtk::common::Managers::Ptr& managers = nullptr);

  ~FillOutAttributes() override = default;

  void configure(const Configuration& config);

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
  State computeInternalState() const;

  smtk::common::Managers::Ptr m_managers;
  smtk::operation::Observers::Key m_observer;
  std::vector<AttributeSet> m_attributeSets;
};
} // namespace task
} // namespace smtk

#endif // smtk_task_FillOutAttributes_h
