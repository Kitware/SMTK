//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_qt_job_ShellQueue_h
#define smtk_qt_job_ShellQueue_h

#include "smtk/common/Managers.h"
#include "smtk/extension/qt/Exports.h" // For export macro.
#include "smtk/job/DatabaseQueue.h"

#include <QObject>
#include <QProcessEnvironment>

#include <filesystem>
#include <memory>
#include <vector>

namespace smtk
{
namespace qt
{
namespace job
{

///\brief ShellQueue schedules jobs locally by immediately running them.
///
/// This class depends on Qt for process execution and timer-based progress polling.
class SMTKQTEXT_EXPORT ShellQueue
  : public QObject
  , public smtk::job::DatabaseQueue
{
  Q_OBJECT
public:
  smtkTypeMacro(smtk::qt::job::ShellQueue);
  smtkSuperclassMacro(smtk::job::DatabaseQueue);
  smtkCreateMacro(smtk::job::Queue);
  smtkSharedFromThisMacro(smtk::job::Queue);

  ShellQueue();
  ShellQueue(
    const smtk::common::UUID& uid,
    const std::shared_ptr<smtk::resource::Manager>& resourceManager);
  ~ShellQueue() override;

  std::string location() const override { return "localhost"; }

  /// This type of queue **may** allow some jobs to be canceled.
  bool allowsCancellation() const override { return true; }

  /// Run the given job as a separate process.
  bool schedule(const std::shared_ptr<smtk::job::Job>& job) override;
  /// Cancel a scheduled job (whether it is running or not).
  bool cancel(const std::shared_ptr<smtk::job::Job>& job) override;
  /// Return the state of a job.
  smtk::job::State jobState(const std::shared_ptr<smtk::job::Job>& job) override;
  /// Return the completion-status of a job.
  smtk::job::Status jobStatus(const std::shared_ptr<smtk::job::Job>& job) override;

  /// Return the set of all jobs in this queue.
  std::set<std::shared_ptr<smtk::job::Job>> allJobs() const override;

  /// Set/get an optional interpreter used to run job scripts.
  ///
  /// When empty (the default), the job script is executed directly. This is
  /// the traditional behavior on Unix. On Windows, callers can set this to a
  /// Bash executable (for example, the Bash bundled with OpenFOAM) so that
  /// Unix shell scripts do not need a native executable suffix or file mode.
  void setInterpreter(const std::filesystem::path& interpreter);
  std::filesystem::path interpreter() const;

  /// Set/get arguments inserted between the interpreter and the job script.
  void setInterpreterArguments(const std::vector<std::string>& arguments);
  std::vector<std::string> interpreterArguments() const;

  /// Set/get the environment inherited by newly launched jobs.
  void setProcessEnvironment(const QProcessEnvironment& environment);
  QProcessEnvironment processEnvironment() const;

protected Q_SLOTS:
  /// Register persisted running jobs and poll them for progress updates.
  virtual void updateJobStates();

private:
  /// Terminate the process tree for \a job. Must run on this object's thread.
  bool cancelProcess(const std::shared_ptr<smtk::job::Job>& job);

  class Internal;
  std::unique_ptr<Internal> m_p;
};

} // namespace job
} // namespace qt
} // namespace smtk

#endif // smtk_qt_job_ShellQueue_h
