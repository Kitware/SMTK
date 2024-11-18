//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_GatherResources_h
#define smtk_task_GatherResources_h

#include "smtk/resource/Manager.h"
#include "smtk/resource/Observer.h"
#include "smtk/resource/Resource.h"
#include "smtk/task/AgentlessTask.h"

#include "smtk/common/Visit.h"

namespace smtk
{
namespace task
{

/**\brief GatherResources is a task that requires resources from a resource manager.
  *
  * Tasks of this type observe a resource manager for resources to be added and marked
  * with a Role as specified (at construction time). When all the required resources
  * are present with the required roles, then the task becomes completable.
  */
class SMTKCORE_EXPORT GatherResources : public AgentlessTask
{
public:
  smtkTypeMacro(smtk::task::GatherResources);
  smtkSuperclassMacro(smtk::task::AgentlessTask);
  smtkCreateMacro(smtk::resource::PersistentObject);

  /// A predicate used to collect resources that fit a given role.
  struct ResourceSet
  {
    using Entry = std::weak_ptr<smtk::resource::Resource>;
    /// The required role. If empty, any role is allowed.
    std::string m_role;
    /// The minimum number of resources that must be collected to satisfy the requirement.
    ///
    /// If 0, then resources selected by the validator are not required (but *may* be accepted).
    unsigned int m_minimumCount;
    /// The maximum number of resources that can be collected while still satisfying the requirement.
    ///
    /// Note that if 0, the resourceSet is forcing the task to reject all resources
    /// that the validator selects (i.e., no resources of the given type are allowed).
    /// If negative, then there is no maximum number of validated resources.
    int m_maximumCount;
    /// The resource typename regex; typically just a resource typename.
    std::string m_type;
    /// A lambda used to determine whether the given resource is acceptable.
    std::function<bool(const smtk::resource::Resource&, const ResourceSet&)> m_validator;
    /// The set of resources being managed that are selected by the validator.
    std::set<Entry, std::owner_less<Entry>> m_resources;
  };
  /// Signature of functors that visit resources-by-role predicates.
  using ResourceSetVisitor = std::function<smtk::common::Visit(const ResourceSet&)>;

  GatherResources();
  GatherResources(
    const Configuration& config,
    Manager& taskManager,
    const smtk::common::Managers::Ptr& managers = nullptr);
  GatherResources(
    const Configuration& config,
    const PassedDependencies& dependencies,
    Manager& taskManager,
    const smtk::common::Managers::Ptr& managers = nullptr);

  ~GatherResources() override = default;

  void configure(const Configuration& config);

  /// Explicitly add a \a resource in the given \a role.
  ///
  /// If \a autoconfigure is disabled, calling this method is
  /// the only way to complete this task.
  ///
  /// The \a role must be one this task is configured to accept.
  /// This method will add the \a resource as long as it is of
  /// the correct type (regardless of whether it is marked with
  /// the \a role passed to this method).
  ///
  /// If the resource was added (i.e., not already present in
  /// the given \a role), then this method returns true.
  bool addResourceInRole(
    const std::shared_ptr<smtk::resource::Resource>& resource,
    const std::string& role);

  smtk::common::Visit visitResourceSets(ResourceSetVisitor visitor);

  bool autoConfigure() const { return m_autoconfigure; }

protected:
  /// Respond to resource changes that may change task state.
  void updateResources(smtk::resource::Resource& resource, smtk::resource::EventType event);

  /// Check m_resourcesByRole to see if all requirements are met.
  State computeInternalState() const override;

  bool m_autoconfigure = false;
  smtk::common::Managers::Ptr m_managers;
  smtk::resource::Observers::Key m_observer;
  std::map<std::string, ResourceSet> m_resourcesByRole;
};
} // namespace task
} // namespace smtk

#endif // smtk_task_GatherResources_h
