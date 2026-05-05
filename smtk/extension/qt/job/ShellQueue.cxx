//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/job/ShellQueue.h"

#include "smtk/job/Job.h"
#include "smtk/job/Queue.h"

#include <QPointer>
#include <QProcess>

#include <cstdlib> // For std::system

namespace smtk
{
namespace qt
{
namespace job
{

using namespace smtk::job;

class ShellQueue::Internal
{
public:
  Internal(ShellQueue* self)
    : m_self(self)
  {
  }

  ShellQueue* m_self{ nullptr };
};

// ShellQueue::ShellQueue(const std::shared_ptr<smtk::common::Managers>& applicationContext);
ShellQueue::ShellQueue()
  : m_p(new ShellQueue::Internal(this))
{
  // TODO: Deserialize local jobs from storage?
}

ShellQueue::~ShellQueue() {}

bool ShellQueue::schedule(const std::shared_ptr<smtk::job::Job>& job)
{
  if (!job || !job->queueId().empty())
  {
    return false;
  }
  if (job->queue() && job->queue() != this)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Cannot submit job to different queue.");
    return false;
  }
  // If the job is not parented by the queue, do so.
  QProcess proc;
  proc.setProgram(QString::fromStdString((job->caseDirectory() / job->script()).string()));
  qint64 pp;
  proc.startDetached(&pp);
  proc.waitForFinished(-1);
  job->setQueueId(std::to_string(pp));
  // TODO: serialize to disk
  return true;
}

bool ShellQueue::cancel(const std::shared_ptr<smtk::job::Job>& job)
{
  if (!job || job->queue() != this || job->queueId().empty())
  {
    return false;
  }
  std::ostringstream cmd;
  cmd << "kill -9 " << job->queueId();
  std::system(cmd.str().c_str());
  // TODO: reset job, removing queueId and queue.
  return true;
}

State ShellQueue::jobState(const std::shared_ptr<smtk::job::Job>& job)
{
  if (!job || job->queue() != this || job->queueId().empty())
  {
    return State::Unscheduled;
  }
  // TODO: Run "ps" to get state of job->queueId().
  return State::Completed;
}

Status ShellQueue::jobStatus(const std::shared_ptr<smtk::job::Job>& job)
{
  if (!job || job->queue() != this)
  {
    return Status::Pending;
  }
  return job->queueId().empty() ? Status::Succeeded : Status::Pending;
}

std::set<std::shared_ptr<smtk::job::Job>> ShellQueue::allJobs() const
{
  std::set<std::shared_ptr<smtk::job::Job>> result;
  return result;
}

} // namespace job
} // namespace qt
} // namespace smtk
