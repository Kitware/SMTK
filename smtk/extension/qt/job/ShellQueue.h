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
#include "smtk/job/Queue.h"

#include <QObject>

#include <memory>

namespace smtk
{
namespace qt
{
namespace job
{

///\brief ShellQueue schedules jobs locally by immediately running them.
///
/// This class depends on Qt for process and filesystem monitoring.
class SMTKQTEXT_EXPORT ShellQueue : public smtk::job::Queue
{
public:
  smtkTypeMacro(smtk::qt::job::ShellQueue);
  smtkSuperclassMacro(smtk::job::Queue);
  smtkCreateMacro(smtk::job::Queue);
  smtkSharedFromThisMacro(smtk::job::Queue);

  ~ShellQueue() override;

  std::string location() const override { return "localhost"; }

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

protected:
  ShellQueue();

private:
  class Internal;
  std::unique_ptr<Internal> m_p;
};

} // namespace job
} // namespace qt
} // namespace smtk

#endif // smtk_qt_job_ShellQueue_h
