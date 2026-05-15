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

#include "smtk/job/Definition.h"
#include "smtk/job/Job.h"
#include "smtk/job/Queue.h"
#include "smtk/job/operators/JobUpdated.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"

#include <QFileSystemWatcher>
#include <QPointer>
#include <QProcess>

#include <cstdlib> // For std::system
#include <fstream>

namespace smtk
{
namespace qt
{
namespace job
{

using namespace smtk::job;
using namespace smtk::string::literals;

class ShellQueue::Internal
{
public:
  Internal(ShellQueue* self)
    : m_self(self)
    , m_watcher(new QFileSystemWatcher)
  {
    QObject::connect(
      m_watcher.data(), &QFileSystemWatcher::fileChanged, self, &ShellQueue::fileUpdated);
    QObject::connect(
      m_watcher.data(), &QFileSystemWatcher::directoryChanged, self, &ShellQueue::directoryUpdated);
  }

  using PathUpdateResponder = std::function<void(const QString&)>;

  bool addPathDispatch(const QString& path, PathUpdateResponder function)
  {
    auto it = m_dispatch.find(path);
    if (it != m_dispatch.end())
    {
      std::cerr << "WARNING: Replacing a dispatch for path (" << path.toStdString() << ").\n";
      it->second = function;
    }
    else
    {
      m_dispatch[path] = function;
    }
    bool didAdd = m_watcher->addPath(path);
    return didAdd;
  }

  void dispatchUpdate(const QString& path)
  {
    auto it = m_dispatch.find(path);
    if (it != m_dispatch.end())
    {
      it->second(path);
    }
  }

  ShellQueue* m_self{ nullptr };
  QScopedPointer<QFileSystemWatcher> m_watcher;
  std::unordered_map<QString, std::function<void(const QString&)>> m_dispatch;
};

// ShellQueue::ShellQueue(const std::shared_ptr<smtk::common::Managers>& applicationContext);
ShellQueue::ShellQueue()
  : m_p(new ShellQueue::Internal(this))
{
  // TODO: Deserialize local jobs from storage?
}

ShellQueue::ShellQueue(
  const smtk::common::UUID& uid,
  const std::shared_ptr<smtk::resource::Manager>& resourceManager)
  : Superclass(uid, resourceManager)
  , m_p(new ShellQueue::Internal(this))
{
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
  if (!this->find(job->id()))
  {
    if (!this->add(job))
    {
      return false;
    }
  }
  QProcess proc;
  proc.setProgram(QString::fromStdString((job->caseDirectory() / job->script()).string()));
  proc.setWorkingDirectory(QString::fromStdString(job->caseDirectory().string()));
  m_p->addPathDispatch(
    QString::fromStdString((job->caseDirectory() / "logs").string()),
    [job, this](const QString& path) {
      if (path.endsWith("logs"))
      {
        // Watch "progress" inside this dir.
        m_p->m_watcher->addPath(path + "/progress");
      }
    });
  m_p->addPathDispatch(
    QString::fromStdString((job->caseDirectory() / "logs" / "progress").string()),
    [job, this](const QString& path) {
      if (path.endsWith("progress"))
      {
        std::ifstream pp(path.toStdString().c_str());
        int stage = -3;
        pp >> stage;
        if (pp.good() && stage > -3)
        {
          // std::cerr << "  Stage " << stage << "\n";
          if (auto operationManager = this->operationManager())
          {
            auto updater = operationManager->create<smtk::job::JobUpdated>();
            updater->parameters()->associate(job);
            updater->parameters()->findInt("stage")->setIsEnabled(true);
            updater->parameters()->findInt("stage")->setValue(stage);
            updater->parameters()->findInt("state")->setIsEnabled(true);
            updater->parameters()->findInt("state")->setValue(
              stage < 0 ? static_cast<int>(smtk::job::State::Scheduled)
                : stage < job->jobType()->stages().size()
                ? static_cast<int>(smtk::job::State::Running)
                : static_cast<int>(smtk::job::State::Completed));
            if (stage < 0)
            {
              updater->parameters()->findInt("status")->setIsEnabled(true);
              updater->parameters()->findInt("status")->setValue(
                static_cast<int>(smtk::job::Status::Pending));
            }
            else if (stage == job->jobType()->stages().size())
            {
              updater->parameters()->findInt("status")->setIsEnabled(true);
              updater->parameters()->findInt("status")->setValue(
                static_cast<int>(smtk::job::Status::Succeeded));
            }
            operationManager->launchers()(updater);
          }
        }
      }
    });
#if 0
  {
    // For debugging:
    std::cerr << "  Watcher Inventory\n";
    for (const auto& dir : m_p->m_watcher->directories())
    {
      std::cerr << "    " << dir.toStdString() << " (dir)\n";
    }
    for (const auto& file : m_p->m_watcher->files())
    {
      std::cerr << "    " << file.toStdString() << " (file)\n";
    }
  }
#endif

  qint64 pp;
  proc.startDetached(&pp);
  // For debugging: Wait for the process to start (but not finish).
  // proc.waitForStarted(-1);
  job->setStage(-1);
  job->setStatus(smtk::job::Status::Pending);
  job->setQueueId(std::to_string(pp));
  job->setState(smtk::job::State::Running);
  // Serialize to the database.
  this->updateJobDatabaseInfo(job);
  return true;
}

bool ShellQueue::cancel(const std::shared_ptr<smtk::job::Job>& job)
{
  if (!job || job->queue() != this || job->queueId().empty())
  {
    return false;
  }
  auto jobState = job->state();
  if (jobState != State::Scheduled && jobState != State::Running)
  {
    // Do not cancel Unscheduled or already-Canceled jobs.
    return false;
  }
  if (jobState == State::Running)
  {
    std::ostringstream cmd;
    cmd << "kill -9 " << job->queueId();
    int killResult = std::system(cmd.str().c_str());
    // std::cerr << "killed? " << killResult << "\n";
    if (!killResult)
    {
      job->setState(State::Canceled);
      job->setStatus(Status::Terminated);
    }
    else
    {
      // Failed to terminate process… probably because the job completed
      // before we could kill it.
      return false;
    }
  }
  else
  {
    // We don't really support scheduled but non-running jobs for this queue type.
    job->setState(State::Canceled);
    job->setStatus(Status::Pending);
  }
  this->updateJobDatabaseInfo(job);
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

void ShellQueue::fileUpdated(const QString& path)
{
  // std::cerr << "File Path \"" << path.toStdString() << "\" updated.\n";
  m_p->dispatchUpdate(path);
}

void ShellQueue::directoryUpdated(const QString& path)
{
  // std::cerr << "Directory Path \"" << path.toStdString() << "\" updated.\n";
  m_p->dispatchUpdate(path);
}

} // namespace job
} // namespace qt
} // namespace smtk
