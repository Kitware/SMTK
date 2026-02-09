//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_job_Job_h
#define smtk_job_Job_h

#include "smtk/job/LogParser.h"
#include "smtk/job/State.h"
#include "smtk/job/Status.h"
#include "smtk/resource/Component.h"
#include "smtk/resource/GuardedLinks.h"

#include <filesystem>
#include <memory>
#include <unordered_map>

namespace smtk
{
namespace job
{

class Queue;

/// A job is a component used to track the progress of computational work (as opposed
/// to interactive tasks handled by the smtk::task subsystem).
///
/// # Overview
///
/// Jobs are modeled with several pieces of information:
/// + jobs are run at a *location* (i.e., a place that has a queue managing the assignment
///   of jobs to computation resources);
/// + jobs are *submitted* to a *queue* for scheduling on compute-resources (i.e., a computer
///   or cluster of computers);
/// + jobs have a *state* (unscheduled, scheduled, cancedled, running, or completed) and
///   a *status* (success or failure if the state is completed and pending otherwise);
/// + jobs generate events at state changes which may be observed on the GUI/main thread;
/// + jobs have a launcher used by the job script to start the process(es) that do the
///   computational work. Usually, this is mpiexec but aprun or srun are alternatives.
///   The queuing system specifies the launcher.
/// + jobs have a concurrency (the number of parallel processes to use when performing work).
///   This may be specified by the operation which creates the job and may be restricted by
///   the queueing system. It should be embedded in the job script. If unspecified, the
///   job location (queue) should provide a concurrency.
/// + jobs represent a collection of files in a "case" directory that include a script
///   which runs one or more executables that process the files. Generally, any output
///   will also be written to the same case directory as the input files.
///   A special "logs/progress" file in a job's case directory is expected to be
///   created/overwritten at job start and be updated as the job proceeds; if it exists,
///   it is used to provide updates to any observers monitoring the job.
///   It is a text file starting with an integer
///   that indicates the current stage of the job. Any text following the integer should
///   be a user-presentable message indicating progress.
/// + jobs may create log files in their case directory when running;
///   + job logs may be assigned a parser to extract information from the log (e.g., simulation
///     convergence metrics; simulated time and time step).
///   + job logs may be presented to the user as they are written;
///   + queueing systems for remote jobs are expected to provide access to log file text and
///     extracted information on demand (though this is not always possible before job completion).
///
/// # Creating jobs
///
/// Call smtk::job::Job::create() inside an operation to create a blank job.
/// Then use the job's methods to prepare it.
///
/// If your operation can determine the queue to which it should be added, you
/// may set the queue. If you do not provide a queue, users will need to manually
/// schedule the job.
///
/// Finally, append the job to the operation result's "jobs" item.
/// As the operation's result is passed to observers,
/// it will be added to the job::Resource::instance(). If a queue is provided
/// by the operation, it will be immediately scheduled to run on this queue.
/// Otherwise, users must manually queue the job.
class SMTKCORE_EXPORT Job : public smtk::resource::Component
{
public:
  smtkTypeMacro(smtk::job::Job);
  smtkCreateMacro(smtk::job::Job);
  smtkSuperclassMacro(smtk::resource::Component);
  smtkSharedFromThisMacro(smtk::resource::PersistentObject);

  /// The type of map used to store parsers for the various log files.
  using LogParserMap = std::unordered_map<smtk::string::Token, LogParser>;

  /// Use a mutex to guard access to links on jobs.
  using GuardedLinks = smtk::resource::GuardedComponentLinks;

  /// Destroy a job (from memory, but not from persistent storage if the job
  /// is owned by the job::Resource).
  ~Job() override;

  /// Either null (during the operation which created the job) or a reference
  /// to smtk::job::Resource::instance() (after the operation's observers are
  /// called).
  const smtk::resource::ResourcePtr resource() const override;

  ///@{
  /// Get/set the UUID of this job.
  const common::UUID& id() const override;
  bool setId(const common::UUID& uid) override;
  ///@}

  ///@{
  /// Set/get the queue on which to schedule this job (or on which it is scheduled).
  Queue* queue() const;
  bool setQueue(Queue*);
  ///@}

  ///@{
  /// Set/get the job size.
  ///
  /// The job size is the maximum concurrency (i.e., number of parallel processes)
  /// the job should use.
  ///
  /// This number is passed to the job script as "-n <size>".
  ///
  /// If you specify a number larger than a queue allows, this value will be changed
  /// when schedule() is called.
  ///
  /// For example, if you call `job->setSize(10)` but then `job->schedule(queue)` with
  /// a `queue` that has a limit of 8 processes per job, then "-n 8" will be passed to
  /// the job's script.
  std::uint64_t size() const;
  bool setSize(std::uint64_t jobSize);
  ///@}

  ///@{
  /// Set/get the directory holding job input and output data.
  std::filesystem::path caseDirectory() const;
  bool setCaseDirectory(std::filesystem::path dir);
  ///@}

  ///@{
  /// Set/get the path to the job's run script. This path must be relative to the case directory.
  std::filesystem::path script() const;
  bool setScript(std::filesystem::path dir);
  ///@}

  ///@{
  /// Set/get a set of log files output during the course of running the job.
  ///
  /// This set need not include the special "logs/progress" file (relative to the case directory)
  /// which is always monitored if it exists. The logs listed here are assumed to contain
  /// human-readable data that user interfaces may wish to present.
  const std::vector<std::filesystem::path>& logs() const;
  bool setLogs(const std::vector<std::filesystem::path>& logFiles);
  ///@}

  ///@{
  /// Set/get the queueing-system-specific job ID (if one exists).
  ///
  /// Until the job status is scheduled, this will return an empty string.
  /// Usually, the string may be turned into a number, but not always.
  std::string queueId() const;
  bool setQueueId(const std::string& queueId);
  ///@}

  /// If the job is not currently queued, queue it.
  ///
  /// If no \a queue is specified, use the job's current queue.
  /// If there is no currently-assigned or specified \a queue, this method
  /// will return false.
  bool schedule(Queue* queue = nullptr);

  /// Return the state of the job relative to the queue.
  State state() const;

  /// Return the completion status of the job.
  Status status() const;

  ///@{
  /// Set/get whether the job should be automatically scheduled when added
  /// to the job::Resource.
  ///
  /// Any operation that reports a new Job instance causes those
  /// jobs to be added to the job::Resource (this work is performed
  /// by an operation observer that is part of the job::Registrar).
  /// If this member is set to true (the default), the the job will
  /// not only be added to the job::Resource instance but also
  /// scheduled on its queue.
  bool autoSchedule() const { return m_autoSchedule; }
  bool setAutoSchedule(bool shouldSchedule);
  ///@}

  ///@{
  /// Manage log-file parsers.
  ///
  /// The \a logPath parameter is relative path from the case directory to the log file.
  /// Log files must live inside their case directories.
  const LogParserMap& logParsers() const;
  bool setLogParser(smtk::string::Token logPath, const LogParser& parser);
  bool clearLogParser(smtk::string::Token logPath);
  bool resetLogParsers();
  ///@}

  using LinkKey = std::pair<smtk::common::UUID, smtk::common::UUID>;

  /// Link this job to the given \a object.
  LinkKey linkTo(const std::shared_ptr<smtk::resource::PersistentObject>& object);

  /// Unlink the \a object from this job.
  bool unlink(LinkKey key);

  /// Return object(s) which originated this job.
  std::set<std::shared_ptr<smtk::resource::PersistentObject>> originators() const;

  // Jobs are used outside of an operation context, where they are not guarded
  // from concurrency issues. Specifically, jobs use links to reference persistent
  // objects associated with a job (such as the task to which the job belongs).
  // This API ensures thread safety when manipulating smtk::job::{Resource,Job}
  // links.
  const GuardedLinks guardedLinks() const;
  GuardedLinks guardedLinks();

  ///@{
  /// Set/get the container image to use for this job.
  ///
  /// This string is a URL indicating the host serving containers
  /// as well as the name and version of the container on that host.
  ///
  /// This is unused unless a queue requires it.
  /// You cannot submit a job without a container image URL to a queue
  /// that uses containers; however, you can submit a job with a
  /// container image URL to a queue that does not use containers – this
  /// setting will just be ignored.
  std::string containerImage() const { return m_containerImage; }
  bool setContainerImage(const std::string& imageURL);
  ///@}

  ///@{
  /// Set/get the location within the image of where to mount the case directory.
  std::string caseDirectoryMountPoint() const { return m_caseDirectoryMountPoint; }
  bool setCaseDirectoryMountPoint(const std::string& mountPoint);
  ///@}

protected:
  Job();

  smtk::common::UUID m_id;
  Queue* m_queue{ nullptr };
  std::uint64_t m_size{ 1 };
  std::string m_queueId;
  std::filesystem::path m_caseDirectory;
  std::filesystem::path m_script;
  std::vector<std::filesystem::path> m_logs;
  bool m_autoSchedule{ true };
  LogParserMap m_logParsers;
  std::string m_containerImage;
  std::string m_caseDirectoryMountPoint;
};

} // namespace job
} // namespace smtk

#endif // smtk_job_Job_h
