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
#include "smtk/extension/qt/job/ProgressMonitor.h"

#include "smtk/job/Definition.h"
#include "smtk/job/Job.h"
#include "smtk/job/Queue.h"
#include "smtk/job/operators/JobUpdated.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Observer.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"

#include <QPointer>
#include <QProcess>
#include <QThread>

#include <atomic>
#include <cerrno>
#include <charconv>
#include <fstream>

#if defined(Q_OS_WIN)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <csignal>
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
    , m_progressMonitor(self, self, 100, [self]() { self->updateJobStates(); })
  {
    if (auto resourceManager = self->manager())
    {
      m_resourceObserver = resourceManager->observers().insert(
        [this](const smtk::resource::Resource&, smtk::resource::EventType event) {
          if (event == smtk::resource::EventType::ADDED)
          {
            // A restored project may satisfy persisted job-origin links. Queue
            // the retry because resource-manager observers must not modify it.
            m_restoreLinks = true;
            QMetaObject::invokeMethod(m_self, &ShellQueue::updateJobStates, Qt::QueuedConnection);
          }
        },
        /* priority */ 0,
        /* initialize */ false,
        "ShellQueue restores job origin links when resources are loaded.");
    }
  }

  ShellQueue* m_self{ nullptr };
  ProgressMonitor m_progressMonitor;
  smtk::resource::Observers::Key m_resourceObserver;
  std::atomic_bool m_restoreLinks{ true };
  std::filesystem::path m_interpreter;
  std::vector<std::string> m_interpreterArguments;
  QProcessEnvironment m_processEnvironment{ QProcessEnvironment::systemEnvironment() };
  std::unordered_map<smtk::common::UUID, QPointer<QProcess>> m_activeProcesses;
#if defined(Q_OS_WIN)
  // A Windows Job Object makes explicit cancellation apply to the complete
  // process tree. It intentionally does not use KILL_ON_JOB_CLOSE: simulations
  // must survive application shutdown.
  std::unordered_map<smtk::common::UUID, HANDLE> m_windowsJobObjects;
#endif
};

// ShellQueue::ShellQueue(const std::shared_ptr<smtk::common::Managers>& applicationContext);
ShellQueue::ShellQueue()
  : m_p(new ShellQueue::Internal(this))
{
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
  // QProcess::~QProcess terminates a process that is still running. Detach the
  // wrappers before QObject child destruction so simulations keep running when
  // SMTK exits. The operating system reclaims these wrapper objects at exit.
  for (auto& entry : m_p->m_activeProcesses)
  {
    if (entry.second)
    {
      entry.second->disconnect(this);
      entry.second->setParent(nullptr);
    }
  }
  m_p->m_activeProcesses.clear();
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
  // Operations may request scheduling from a worker thread. QProcess and the
  // progress monitor's QTimer must only be used from their owning QObject thread.
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
  // Do not leave the child connected to QProcess-owned pipes: those handles
  // disappear when SMTK exits and can cause a surviving child to receive a
  // broken pipe. Files remain valid independently of the parent process.
  auto logsDirectory = job->caseDirectory() / "logs";
  std::filesystem::create_directories(logsDirectory);
  proc->setStandardOutputFile(
    QString::fromStdString((logsDirectory / "shell-queue.stdout.log").string()), QIODevice::Append);
  proc->setStandardErrorFile(
    QString::fromStdString((logsDirectory / "shell-queue.stderr.log").string()), QIODevice::Append);
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
  m_p->m_progressMonitor.start(job.get());

  m_p->m_activeProcesses[job->id()] = proc;
  QObject::connect(proc, &QProcess::started, this, [this, job, proc]() {
    job->setQueueId(std::to_string(proc->processId()));
#if defined(Q_OS_WIN)
    // Descendants inherit Job Object membership, making explicit cancellation
    // reliable for shell-launched tools. Do not enable KILL_ON_JOB_CLOSE;
    // closing SMTK must not terminate a long-running simulation.
    HANDLE processHandle = OpenProcess(
      PROCESS_SET_QUOTA | PROCESS_TERMINATE | PROCESS_QUERY_LIMITED_INFORMATION,
      FALSE,
      static_cast<DWORD>(proc->processId()));
    HANDLE jobHandle = CreateJobObjectW(nullptr, nullptr);
    if (processHandle && jobHandle)
    {
      if (AssignProcessToJobObject(jobHandle, processHandle))
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
          // Capture the last progress value before marking the job completed;
          // the periodic monitor intentionally ignores terminal jobs.
          std::ifstream progress(job->caseDirectory() / "logs" / "progress");
          int stage = -3;
          progress >> stage;
          if (progress.good() && stage > job->stage())
          {
            updater->parameters()->findInt("stage")->setIsEnabled(true);
            updater->parameters()->findInt("stage")->setValue(stage);
          }
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
  QProcess* process =
    processIt == m_p->m_activeProcesses.end() ? nullptr : processIt->second.data();
  qint64 processId = 0;
  const auto& queueId = job->queueId();
  auto parseResult = std::from_chars(queueId.data(), queueId.data() + queueId.size(), processId);
  if (
    parseResult.ec != std::errc() || parseResult.ptr != queueId.data() + queueId.size() ||
    processId <= 0)
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
  if (!terminated)
  {
    // This is also the recovery path after restarting SMTK, when the persisted
    // PID remains available but the original Job Object handle does not.
    QProcess taskkill;
    taskkill.start("taskkill.exe", { "/PID", QString::number(processId), "/T", "/F" });
    terminated =
      taskkill.waitForStarted(3000) && taskkill.waitForFinished(10000) && taskkill.exitCode() == 0;
  }
#else
  if (process)
  {
    process->terminate();
    terminated = process->waitForFinished(3000);
  }
  else
  {
    // A restored queue has no QProcess wrapper, but the persisted PID still
    // permits cancellation with the behavior used by the original ShellQueue.
    terminated = ::kill(static_cast<pid_t>(processId), SIGTERM) == 0;
  }
#endif
  if (!terminated && process)
  {
    // QProcess::kill is a last-resort fallback. On Windows the Job Object path
    // above is preferred because kill() alone does not terminate descendants.
    process->kill();
    terminated = process->waitForFinished(3000);
  }
  return terminated;
}

State ShellQueue::jobState(const std::shared_ptr<smtk::job::Job>& job)
{
  if (!job || job->queue() != this)
  {
    return State::Unscheduled;
  }
  return job->state();
}

Status ShellQueue::jobStatus(const std::shared_ptr<smtk::job::Job>& job)
{
  if (!job || job->queue() != this)
  {
    return Status::Pending;
  }
  return job->status();
}

std::set<std::shared_ptr<smtk::job::Job>> ShellQueue::allJobs() const
{
  return this->Superclass::allJobs();
}

void ShellQueue::updateJobStates()
{
  bool restoreLinks = m_p->m_restoreLinks.exchange(false);
  for (const auto& job : this->Superclass::allJobs())
  {
    if (restoreLinks)
    {
      // Projects and tasks are commonly loaded after queues. Resource-manager
      // additions request this retry so persisted origin links reconnect once.
      this->fetchJobLinks(job);
    }
    if (job->state() != State::Scheduled && job->state() != State::Running)
    {
      continue;
    }
    m_p->m_progressMonitor.start(job.get());
  }
  m_p->m_progressMonitor.update();
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

} // namespace job
} // namespace qt
} // namespace smtk
