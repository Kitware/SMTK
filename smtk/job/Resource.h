//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_job_Resource_h
#define smtk_job_Resource_h

#include "smtk/resource/DerivedFrom.h"
#include "smtk/resource/GuardedLinks.h"
#include "smtk/resource/Resource.h"

#include "smtk/job/Job.h"

#include <memory>

namespace smtk
{
/// A subsystem for managing asynchronous, long-running computational work that
/// may be either local or remote.
namespace job
{

/// A singleton resource for managing (potentially) long-running computational jobs.
///
/// Jobs are the components this resource owns.
///
/// Computational jobs may run locally or remotely (with status information forwarded
/// on demand by clients).
///
/// You should never directly modify this resource or obtain a lock on this resource.
/// Interact with it by posting operation results to add jobs and use observers it
/// provides to monitor job state changes (on the main/GUI thread).
///
/// For now, jobs may not be deleted (though their artifacts such as logs and simulation
/// output may be removed). In the future, some mechanism for purging non-running jobs
/// (i.e., canceled, never-queued, or completed) may be provided.
///
/// ## The Job Lifecycle
///
/// Jobs are added to the resource by placing them in operation results
/// (as referenced components); any job mentioned in an operation result's
/// "jobsToAdd" item will be automatically added to this resource if not
/// already present. Because no operation should lock this resource singleton,
/// it can be considered as always locked by the GUI thread. Other threads
/// (operations) should not attempt to access the resource for either reading
/// or writing unless they use patterns which force their `operateInternal()`
/// method to be invoked on the main/GUI thread (which will obviously need to
/// be short in duration as it will prevent a responsive UI). Python operations
/// do invoke their `operateInternal()` method on the main thread, so they are
/// an easy way to process job data if absolutely needed.
///
/// Once a job is owned by this resource, it may be scheduled on a queue
/// (as specified by the Job). Jobs without a queue or marked to be
/// manually queued will not automatically be submitted for running.
/// Jobs that are queued may be canceled (before or after they are provided
/// an allocation of resources to run on). Jobs may provide progress
/// updates via log files. Once a job is complete, observers on the GUI thread
/// are notified.
///
/// Because jobs run in the background and do not lock SMTK resources
/// (which need not even be present), jobs are considered **volatile**.
/// This means that **jobs may change state outside of operations.**
/// Specifically, the status, progress, and queue-id of a job may be
/// modified outside of operations.
///
/// Unlike other resources, changes to this resource are always flushed to
/// storage (which is unspecified by the public API) immediately.
/// The location of the resource is empty and may not be changed; it is
/// intentionally opaque as the resource *may* federate jobs from multiple
/// sources (e.g., local jobs from a configuration file, remote jobs for
/// each HPC submission location via web or database). Federation allows
/// resources to be shared among users that may have access to the same
/// remote job queues without explicitly transferring job information.
/// In that case, it is up to the resource to manage authentication and
/// authorization for fetching job information.
/// Federation is planned but not currently implemented.
class SMTKCORE_EXPORT Resource
  : public smtk::resource::DerivedFrom<Resource, smtk::resource::Resource>
{
public:
  using GuardedLinks = smtk::resource::GuardedResourceLinks;

  smtkTypeMacro(smtk::job::Resource);
  smtkCreateMacro(smtk::job::Resource);
  smtkSharedFromThisMacro(smtk::resource::PersistentObject);

  Resource(Resource&&) = default;
  ~Resource() override;

  static std::shared_ptr<Resource> instance();

  bool setId(const common::UUID& uid) override { return false; }
  std::string name() const override { return "jobs"; }
  smtk::resource::Resource* parentResource() const override { return nullptr; }

  smtk::resource::ComponentPtr find(const smtk::common::UUID& compId) const override;
  void visit(std::function<void(const smtk::resource::ComponentPtr&)>& v) const override;

  smtk::string::Token templateType() const override;
  std::size_t templateVersion() const override;

  /// Add the given \a job to the resource.
  ///
  /// Jobs are typically created by operations and automatically added
  /// to this resource by an operation observer registered by the job::Registrar.
  bool addJob(const std::shared_ptr<smtk::job::Job>& job);

  /// Retrieve a job component given its UUID
  std::shared_ptr<smtk::job::Job> findJob(const smtk::common::UUID& uid) const;

  // Jobs are used outside of an operation context, where they are not guarded
  // from concurrency issues. Specifically, jobs use links to reference persistent
  // objects associated with a job (such as the task to which the job belongs).
  // This API ensures thread safety when manipulating smtk::job::{Resource,Job}
  // links.
  const GuardedLinks guardedLinks() const;
  GuardedLinks guardedLinks();

  /// Used for thread-safe access to links.
  std::mutex& mutex() const { return m_mutex; }

  /// Link from a job to persistent objects responsible for the job's creation.
  static constexpr smtk::resource::Links::RoleType JobOriginRole = -1;
  static constexpr smtk::resource::Links::RoleType jobOriginRole()
  {
    return Resource::JobOriginRole;
  }

protected:
  friend class Job;

  /// Erase a job from the index (so its UUID/name can be modified).
  ///
  /// This is only called from Job::setId() or Job::setName().
  /// Do not attempt to erase jobs. To remove a job's on-disk data
  /// (such as log files, simulation results, and other artifacts),
  /// call `job->clean();`.
  std::size_t eraseJob(const std::shared_ptr<Job>& job);

  Resource(const smtk::common::UUID& uid);
  // Resource(const smtk::common::UUID& uid, smtk::resource::ManagerPtr manager);
  // Resource(smtk::resource::ManagerPtr manager);
  Resource();
  static smtk::common::UUID singletonId();

private:
  mutable std::mutex m_mutex;
};

} // namespace job
} // namespace smtk

#endif // smtk_job_Resource_h
