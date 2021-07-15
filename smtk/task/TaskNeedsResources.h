//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_TaskNeedsResources_h
#define smtk_task_TaskNeedsResources_h

#include "smtk/resource/Manager.h"
#include "smtk/resource/Observer.h"
#include "smtk/resource/Resource.h"
#include "smtk/task/Task.h"

namespace smtk
{
namespace task
{

/**\brief TaskNeedsResources is a task that requires resources from a resource manager.
  *
  * Tasks of this type observe a resource manager for resources to be added and marked
  * with a Role as specified (at construction time). When all the required resources
  * are present with the required roles, then the task becomes completable.
  */
class SMTKCORE_EXPORT TaskNeedsResources : public Task
{
public:
  smtkTypeMacro(smtk::task::TaskNeedsResources);
  smtkSuperclassMacro(smtk::task::Task);
  smtkCreateMacro(smtk::task::Task);

  /// A predicate used to collect resources that fit a given role.
  struct Predicate
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
    /// Note that if 0, the predicate is forcing the task to reject all resources
    /// that the validator selects (i.e., no resources of the given type are allowed).
    /// If negative, then there is no maximum number of validated resources.
    int m_maximumCount;
    /// The resource typename regex; typically just a resource typename.
    std::string m_type;
    /// A lambda used to determine whether the given resource is acceptable.
    std::function<bool(const smtk::resource::Resource&, const Predicate&)> m_validator;
    /// The set of resources being managed that are selected by the validator.
    std::set<Entry, std::owner_less<Entry>> m_resources;
  };

  TaskNeedsResources();
  TaskNeedsResources(
    const Configuration& config,
    const smtk::common::Managers::Ptr& managers = nullptr);
  TaskNeedsResources(
    const Configuration& config,
    const PassedDependencies& dependencies,
    const smtk::common::Managers::Ptr& managers = nullptr);

  ~TaskNeedsResources() override = default;

  void configure(const Configuration& config);

protected:
  /// Respond to resource changes that may change task state.
  void updateResources(smtk::resource::Resource& resource, smtk::resource::EventType event);

  /// Check m_resourcesByRole to see if all requirements are met.
  State computeInternalState() const;

  smtk::common::Managers::Ptr m_managers;
  smtk::resource::Observers::Key m_observer;
  std::map<std::string, Predicate> m_resourcesByRole;
};
} // namespace task
} // namespace smtk

#endif // smtk_task_TaskNeedsResources_h
