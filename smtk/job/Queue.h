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

#include "smtk/CoreExports.h"
#include "smtk/SharedFromThis.h"
#include "smtk/job/State.h"
#include "smtk/job/Status.h"

#include <memory>
#include <set>
#include <string>
#include <unordered_set>

namespace smtk
{
namespace job
{

class Job;

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
class SMTKCORE_EXPORT Queue : smtkEnableSharedPtr(smtk::job::Queue)
{
public:
  smtkTypeMacroBase(smtk::job::Queue);
  // smtkSharedFromThisMacro(smtk::job::Queue);

  virtual ~Queue() = default;

  /// A user-presentable name for the queue.
  std::string name() const;
  bool setName(const std::string& name);

  /// A user-presentable description of the queue.
  std::string description() const;
  bool setDescription(const std::string& description);

  /// A set of tags that can be used to find relevant queues.
  ///
  /// Each tag (a string token) indicates a capability the queue
  /// provides. Jobs submitted to a queue are expected to have
  /// their tags as a subset of the queue's tags.
  std::unordered_set<smtk::string::Token> tags() const;
  bool addTag(smtk::string::Token tag);
  bool removeTag(smtk::string::Token tag);
  bool hasTag(smtk::string::Token tag) const;

  /// Return the queue's location. Typically, this is the hostname of the login nodes
  /// from which jobs may be queued.
  virtual std::string location() const;

  /// Indicate this queue's maximum job size (concurrent processes/ranks).
  ///
  /// A limit of 0 indicates no restrictions are placed on job size.
  /// Realistically, the limit should always return the number of ranks allowed
  /// across all compute resources unless the queue places stricter limits on jobs.
  virtual std::uint64_t maximumJobSize() const;

  /// Schedule a job to run on the queue's resources.
  virtual bool schedule(const std::shared_ptr<Job>& job);
  /// Cancel a scheduled job (whether it is running or not).
  /// Some schedulers may refuse to allow users to cancel jobs.
  virtual bool cancel(const std::shared_ptr<Job>& job);
  /// Return the state of a job.
  virtual State jobState(const std::shared_ptr<Job>& job);
  /// Return the completion-status of a job.
  virtual Status jobStatus(const std::shared_ptr<Job>& job);

  /// Return the set of all jobs in this queue.
  virtual std::set<std::shared_ptr<Job>> allJobs() const;

  std::string m_name;
  std::string m_description;
  std::unordered_set<smtk::string::Token> m_tags;
};

} // namespace job
} // namespace smtk

#endif // smtk_job_Queue_h
