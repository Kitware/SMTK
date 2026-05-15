//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_job_Queue_h
#define smtk_job_Queue_h

#include "smtk/job/State.h"
#include "smtk/job/Status.h"

#include "smtk/operation/Manager.h"

#include "smtk/resource/DerivedFrom.h"
#include "smtk/resource/GuardedLinks.h"
#include "smtk/resource/Resource.h"

#include "smtk/CoreExports.h"
#include "smtk/SharedFromThis.h"

#include <memory>
#include <set>
#include <string>
#include <unordered_set>

namespace smtk
{
namespace job
{

class Job;
class Manager;

/// A queue is a location where jobs may be scheduled (i.e., assigned computational
/// resources on which to run).
///
/// A queue is responsible for
/// + scheduling the job script (and canceling a scheduled job if requested);
/// + notifying observers of changes to job state and status;
/// + responding to requests for the progress of a job;
/// + managing log-file parsers and the artifacts they produce.
///
/// This class serves as a base class; subclasses may implement support
/// for different schedulers and remote connections.
///
/// This base class provides methods for storing and indexing jobs.
/// It currently uses sqlite3 internally, but that may change.
class SMTKCORE_EXPORT Queue : public smtk::resource::DerivedFrom<Queue, smtk::resource::Resource>
{
public:
  using GuardedLinks = smtk::resource::GuardedResourceLinks;

  smtkTypeMacro(smtk::job::Queue);
  smtkCreateMacro(smtk::job::Queue);
  smtkSharedFromThisMacro(smtk::resource::PersistentObject);

  Queue();
  Queue(const smtk::common::UUID& uid);
  Queue(const smtk::common::UUID& uid, resource::ManagerPtr manager);
  Queue(resource::ManagerPtr manager);
  Queue(Queue&&) = default;
  ~Queue() override = default;

  /// Provide a method to change the UUID of a queue (used for serialization/deserialization)
  bool setId(const common::UUID& uid) override;

  /// A user-presentable name for the queue.
  std::string name() const override;
  bool setName(const std::string& name);

  /// A user-presentable description of the queue.
  std::string description() const;
  bool setDescription(const std::string& description);

  /// Return true if queues of this type allow jobs to be canceled.
  ///
  /// This does not mean that every job instance may be canceled but does mean
  /// that it is possible for the Queue::cancel() method to return true.
  ///
  /// This method is intended for user interfaces to decide whether to provide
  /// users with an affordance to cancel jobs on this queue.
  ///
  /// Subclasses which allow jobs to be canceled should override this method.
  virtual bool allowsCancellation() const { return false; }

  smtk::resource::ComponentPtr find(const smtk::common::UUID& compId) const override;
  void visit(std::function<void(const smtk::resource::ComponentPtr&)>& v) const override;
  std::function<bool(const smtk::resource::Component&)> queryOperation(
    const std::string&) const override;

  smtk::string::Token templateType() const override;
  std::size_t templateVersion() const override;

  /// A set of tags that can be used to find relevant queues.
  ///
  /// Each tag (a string token) indicates a capability the queue
  /// provides. Jobs submitted to a queue are expected to have
  /// their tags as a subset of the queue's tags.
  virtual std::unordered_set<smtk::string::Token> tags() const;
  virtual bool addTag(smtk::string::Token tag);
  virtual bool removeTag(smtk::string::Token tag);
  virtual bool hasTag(smtk::string::Token tag) const;
  virtual bool hasAllTags(const std::unordered_set<smtk::string::Token>& tagSet) const;

  /// Return the queue's location. Typically, this is the hostname of the login nodes
  /// from which jobs may be queued.
  virtual std::string location() const;

  /// Indicate this queue's maximum job size (concurrent processes/ranks).
  ///
  /// A limit of 0 indicates no restrictions are placed on job size.
  /// Realistically, the limit should always return the number of ranks allowed
  /// across all compute resources unless the queue places stricter limits on jobs.
  virtual std::uint64_t maximumJobSize() const;

  /// Retrieve a job component given its UUID
  ///
  /// Unlike the inherited find() method, this version returns a Job.
  virtual std::shared_ptr<smtk::job::Job> findJob(const smtk::common::UUID& uid) const;

  /// Add a job to the queue without scheduling it.
  ///
  /// Do not call this method outside of an operation.
  virtual bool add(const std::shared_ptr<Job>& job);
  /// Schedule a job to run on the queue's resources.
  ///
  /// Do not call this method outside of an operation.
  virtual bool schedule(const std::shared_ptr<Job>& job);
  /// Cancel a scheduled job (whether it is running or not).
  /// Some schedulers may refuse to allow users to cancel jobs.
  ///
  /// Do not call this method outside of an operation.
  virtual bool cancel(const std::shared_ptr<Job>& job);

  /// Return the state of a job.
  virtual State jobState(const std::shared_ptr<Job>& job);
  /// Return the completion-status of a job.
  virtual Status jobStatus(const std::shared_ptr<Job>& job);

  /// Return the set of all jobs in this queue.
  virtual std::set<std::shared_ptr<Job>> allJobs() const;

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
  static constexpr smtk::resource::Links::RoleType jobOriginRole() { return Queue::JobOriginRole; }

  ///@{
  /// Set/get the job manager that is managing this queue.
  ///
  /// This may be used by the queue to fetch job definitions
  /// when restoring jobs from persistent storage.
  ///
  /// It is set by the job::QueueInstances class when a
  /// queue is manage()-ed.
  smtk::job::Manager* jobManager() const { return m_jobManager; }
  void setJobManager(smtk::job::Manager* jobManager);
  ///@}

  ///@{
  /// Set/get the operation manager this queue should use to notify the application of job updates.
  ///
  /// Without an operation manager, the queue may change the job's state but no one will be
  /// notified of the change because the queue needs to launch a JobUpdated operation to ensure
  /// that the queue's resource lock is held while its components (jobs) are modified.
  ///
  /// This should be set when a registrar creates or restores a job queue.
  smtk::operation::Manager::Ptr operationManager() const { return m_operationManager.lock(); }
  void setOperationManager(smtk::operation::Manager::Ptr operationManager);
  ///@}

  /// The signature of functions that receive updates (on the main thread) of
  /// job stage/state/status changes.
  using JobUpdateObserver = std::function<void(const Job& job)>;

  ///@{
  /// Observers notified of changes to a job's state (or all jobs if \a job is null).
  ///
  /// TODO: This should be removed in favor of an "object observer" utility to more
  /// closely match other observers which are handled by smtk::common::Observers<…>.
  /// The difference is that we wish to allow specific objects to be observed rather
  /// than all events for all objects.
  ///
  /// These methods are virtual since observe() will call the inefficient "allJobs()"
  /// method when \a initialize is true and \a job is null.
  ///
  /// The returned value is a key that should be used to unobserve jobs as needed.
  virtual int observe(Job* job, JobUpdateObserver observer, bool initialize);
  virtual void unobserve(Job* job, int key);
  ///@}

protected:
  std::string m_name;
  std::string m_description;
  std::unordered_set<smtk::string::Token> m_tags;
  smtk::job::Manager* m_jobManager{ nullptr };
  std::weak_ptr<smtk::operation::Manager> m_operationManager;
  smtk::operation::Observers::Key m_operationObserverKey;
  std::map<smtk::common::UUID, std::map<int, JobUpdateObserver>> m_observers;
  int m_nextObserverKey{ 1 };

private:
  mutable std::mutex m_mutex;
};

} // namespace job
} // namespace smtk

#endif // smtk_job_Queue_h
