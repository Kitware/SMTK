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
#include <QThread>

#include <fstream>

#if defined(Q_OS_WIN)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

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
  std::filesystem::path m_interpreter;
  std::vector<std::string> m_interpreterArguments;
  QProcessEnvironment m_processEnvironment{ QProcessEnvironment::systemEnvironment() };
  std::unordered_map<smtk::common::UUID, QPointer<QProcess>> m_activeProcesses;
#if defined(Q_OS_WIN)
  // A Windows Job Object makes cancellation apply to the complete process
  // tree (Bash, OpenFOAM solvers, and MPI children), not just to bash.exe.
  std::unordered_map<smtk::common::UUID, HANDLE> m_windowsJobObjects;
#endif
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

ShellQueue::~ShellQueue()
{
#if defined(Q_OS_WIN)
  for (const auto& entry : m_p->m_windowsJobObjects)
  {
    CloseHandle(entry.second);
  }
#endif
}

bool ShellQueue::schedule(const std::shared_ptr<smtk::job::Job>& job)
{
  if (!job || !job->queueId().empty())
  {
    return false;
  }
  // Operations may request scheduling from a worker thread. QProcess and
  // QFileSystemWatcher must only be used from their owning QObject thread.
  // Use a queued handoff rather than a blocking one: SMTK's application thread
  // may be waiting for this worker operation to return.
  if (QThread::currentThread() != this->thread())
  {
    QMetaObject::invokeMethod(
      this, [this, job]() { this->schedule(job); }, Qt::QueuedConnection);
    return true;
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
  auto* proc = new QProcess(this);
  proc->setWorkingDirectory(QString::fromStdString(job->caseDirectory().string()));
  proc->setProcessEnvironment(m_p->m_processEnvironment);
  if (m_p->m_interpreter.empty())
  {
    proc->setProgram(QString::fromStdString((job->caseDirectory() / job->script()).string()));
  }
  else
  {
    proc->setProgram(QString::fromStdString(m_p->m_interpreter.string()));
    QStringList arguments;
    for (const auto& argument : m_p->m_interpreterArguments)
    {
      arguments.append(QString::fromStdString(argument));
    }
    // The child already has the case directory as its working directory. A
    // relative, forward-slash path avoids passing a Windows path through an
    // MSYS Bash command line and also handles case directories with spaces.
    arguments.append("./" + QString::fromStdString(job->script().generic_string()));
    proc->setArguments(arguments);
  }
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

  m_p->m_activeProcesses[job->id()] = proc;
  QObject::connect(proc, &QProcess::started, this, [this, job, proc]() {
    job->setQueueId(std::to_string(proc->processId()));
#if defined(Q_OS_WIN)
    // Assign the new process to a kill-on-close Job Object. Descendants inherit
    // membership, which makes cancellation reliable for shell-launched tools.
    HANDLE processHandle = OpenProcess(
      PROCESS_SET_QUOTA | PROCESS_TERMINATE | PROCESS_QUERY_LIMITED_INFORMATION,
      FALSE,
      static_cast<DWORD>(proc->processId()));
    HANDLE jobHandle = CreateJobObjectW(nullptr, nullptr);
    if (processHandle && jobHandle)
    {
      JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
      limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
      if (
        SetInformationJobObject(
          jobHandle, JobObjectExtendedLimitInformation, &limits, sizeof(limits)) &&
        AssignProcessToJobObject(jobHandle, processHandle))
      {
        m_p->m_windowsJobObjects[job->id()] = jobHandle;
        jobHandle = nullptr; // Ownership transferred to m_windowsJobObjects.
      }
    }
    if (processHandle)
    {
      CloseHandle(processHandle);
    }
    if (jobHandle)
    {
      CloseHandle(jobHandle);
    }
#endif
    this->updateJobDatabaseInfo(job);
  });
  QObject::connect(proc, &QProcess::errorOccurred, this, [this, job](QProcess::ProcessError error) {
    if (error == QProcess::FailedToStart)
    {
      job->setState(smtk::job::State::Completed);
      job->setStatus(smtk::job::Status::Failed);
      this->updateJobDatabaseInfo(job);
    }
  });
  QObject::connect(
    proc,
    qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
    this,
    [this, job, proc](int exitCode, QProcess::ExitStatus exitStatus) {
      // Cancellation has already assigned a more specific terminal state.
      if (job->state() != smtk::job::State::Canceled)
      {
        if (auto operationManager = this->operationManager())
        {
          // Use JobUpdated instead of modifying the Job silently. Besides
          // persistence, this notifies queue observers that the process ended.
          auto updater = operationManager->create<smtk::job::JobUpdated>();
          updater->parameters()->associate(job);
          updater->parameters()->findInt("state")->setIsEnabled(true);
          updater->parameters()->findInt("state")->setValue(
            static_cast<int>(smtk::job::State::Completed));
          updater->parameters()->findInt("status")->setIsEnabled(true);
          updater->parameters()->findInt("status")->setValue(static_cast<int>(
            exitStatus == QProcess::NormalExit && exitCode == 0 ? smtk::job::Status::Succeeded
                                                                : smtk::job::Status::Failed));
          operationManager->launchers()(updater);
        }
        else
        {
          job->setState(smtk::job::State::Completed);
          job->setStatus(
            exitStatus == QProcess::NormalExit && exitCode == 0 ? smtk::job::Status::Succeeded
                                                                : smtk::job::Status::Failed);
          this->updateJobDatabaseInfo(job);
        }
      }
      m_p->m_activeProcesses.erase(job->id());
#if defined(Q_OS_WIN)
      auto handleIt = m_p->m_windowsJobObjects.find(job->id());
      if (handleIt != m_p->m_windowsJobObjects.end())
      {
        CloseHandle(handleIt->second);
        m_p->m_windowsJobObjects.erase(handleIt);
      }
#endif
      proc->deleteLater();
    });

  job->setStage(-1);
  job->setStatus(smtk::job::Status::Pending);
  job->setState(smtk::job::State::Running);
  proc->start();
  if (!proc->waitForStarted())
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Could not start job interpreter \"" << proc->program().toStdString()
                                           << "\": " << proc->errorString().toStdString());
    m_p->m_activeProcesses.erase(job->id());
    proc->deleteLater();
    return false;
  }
  return true;
}

bool ShellQueue::cancel(const std::shared_ptr<smtk::job::Job>& job)
{
  // Active QProcess instances belong to the queue's Qt thread. Queue the
  // request instead of blocking for the same reason described in schedule().
  // The job remains alive through the shared pointer captured by the functor.
  if (QThread::currentThread() != this->thread())
  {
    if (!job || job->queue() != this || job->queueId().empty())
    {
      return false;
    }
    auto jobState = job->state();
    if (jobState != State::Scheduled && jobState != State::Running)
    {
      return false;
    }
    QMetaObject::invokeMethod(
      this, [this, job]() { this->cancelProcess(job); }, Qt::QueuedConnection);
    // CancelJob assigns the Canceled state after this method returns. Assign
    // the terminal status here so its operation result reports both changes.
    job->setStatus(Status::Terminated);
    this->updateJobDatabaseInfo(job);
    return true;
  }
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
    if (!this->cancelProcess(job))
    {
      return false;
    }
    job->setState(State::Canceled);
    job->setStatus(Status::Terminated);
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

bool ShellQueue::cancelProcess(const std::shared_ptr<smtk::job::Job>& job)
{
  auto processIt = m_p->m_activeProcesses.find(job->id());
  if (processIt == m_p->m_activeProcesses.end() || !processIt->second)
  {
    return false;
  }
  bool terminated = false;
#if defined(Q_OS_WIN)
  auto handleIt = m_p->m_windowsJobObjects.find(job->id());
  if (handleIt != m_p->m_windowsJobObjects.end())
  {
    terminated = TerminateJobObject(handleIt->second, ERROR_CANCELLED) != FALSE;
  }
  if (!terminated && processIt->second->processId() > 0)
  {
    // Assignment to a Job Object can be rejected when the parent application
    // is itself constrained by another Job Object. taskkill /T is the fallback
    // that still terminates the complete descendant tree in that situation.
    QProcess taskkill;
    taskkill.start(
      "taskkill.exe", { "/PID", QString::number(processIt->second->processId()), "/T", "/F" });
    terminated =
      taskkill.waitForStarted(3000) && taskkill.waitForFinished(10000) && taskkill.exitCode() == 0;
  }
#else
  processIt->second->terminate();
  terminated = processIt->second->waitForFinished(3000);
#endif
  if (!terminated)
  {
    // QProcess::kill is a last-resort fallback. On Windows the Job Object path
    // above is preferred because kill() alone does not terminate descendants.
    processIt->second->kill();
    terminated = processIt->second->waitForFinished(3000);
  }
  return terminated;
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

void ShellQueue::setInterpreter(const std::filesystem::path& interpreter)
{
  m_p->m_interpreter = interpreter;
}

std::filesystem::path ShellQueue::interpreter() const
{
  return m_p->m_interpreter;
}

void ShellQueue::setInterpreterArguments(const std::vector<std::string>& arguments)
{
  m_p->m_interpreterArguments = arguments;
}

std::vector<std::string> ShellQueue::interpreterArguments() const
{
  return m_p->m_interpreterArguments;
}

void ShellQueue::setProcessEnvironment(const QProcessEnvironment& environment)
{
  m_p->m_processEnvironment = environment;
}

QProcessEnvironment ShellQueue::processEnvironment() const
{
  return m_p->m_processEnvironment;
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
