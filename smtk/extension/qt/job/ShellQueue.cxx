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
#include "smtk/resource/Manager.h"
#include "smtk/resource/Observer.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"

#include <QFileSystemWatcher>
#include <QPointer>
#include <QProcess>
#include <QThread>
#include <QTimer>

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
    , m_watcher(new QFileSystemWatcher)
  {
    QObject::connect(
      m_watcher.data(), &QFileSystemWatcher::fileChanged, self, &ShellQueue::fileUpdated);
    QObject::connect(
      m_watcher.data(), &QFileSystemWatcher::directoryChanged, self, &ShellQueue::directoryUpdated);
    // QFileSystemWatcher notifications can be missed while the application is
    // closed (and when a progress file is replaced). Polling also discovers
    // persisted jobs after a project and its tasks have been restored.
    m_timer.setInterval(1000);
    m_timer.setSingleShot(false);
    QObject::connect(&m_timer, &QTimer::timeout, self, &ShellQueue::updateJobStates);
    m_timer.start();
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

  using PathUpdateResponder = std::function<void(const QString&)>;

  bool addPathDispatch(const QString& path, PathUpdateResponder function)
  {
    auto it = m_dispatch.find(path);
    if (it != m_dispatch.end())
    {
      it->second = function;
    }
    else
    {
      m_dispatch[path] = function;
    }
    if (m_watcher->files().contains(path) || m_watcher->directories().contains(path))
    {
      return true;
    }
    return m_watcher->addPath(path);
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
  QTimer m_timer;
  smtk::resource::Observers::Key m_resourceObserver;
  std::atomic_bool m_restoreLinks{ true };
  std::unordered_map<QString, std::function<void(const QString&)>> m_dispatch;
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
  this->watchJob(job);
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

void ShellQueue::watchJob(const std::shared_ptr<smtk::job::Job>& job)
{
  if (!job)
  {
    return;
  }
  m_p->addPathDispatch(
    QString::fromStdString((job->caseDirectory() / "logs").string()), [this](const QString& path) {
      if (path.endsWith("logs"))
      {
        m_p->m_watcher->addPath(path + "/progress");
      }
    });
  m_p->addPathDispatch(
    QString::fromStdString((job->caseDirectory() / "logs" / "progress").string()),
    [this](const QString&) { this->updateJobStates(); });
}

void ShellQueue::updateJobStates()
{
  auto operationManager = this->operationManager();
  if (!operationManager)
  {
    return;
  }
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
    this->watchJob(job);
    std::ifstream progress(job->caseDirectory() / "logs" / "progress");
    int stage = -3;
    int exitCode = 0;
    progress >> stage;
    if (!progress.good() || stage <= -3 || stage == job->stage())
    {
      continue;
    }
    progress >> exitCode;
    auto updater = operationManager->create<smtk::job::JobUpdated>();
    updater->parameters()->associate(job);
    updater->parameters()->findInt("stage")->setIsEnabled(true);
    updater->parameters()->findInt("stage")->setValue(stage);
    updater->parameters()->findInt("state")->setIsEnabled(true);
    updater->parameters()->findInt("state")->setValue(
      stage < 0 ? static_cast<int>(State::Scheduled)
        : stage < static_cast<int>(job->jobType()->stages().size())
        ? static_cast<int>(State::Running)
        : static_cast<int>(State::Completed));
    if (stage < 0)
    {
      updater->parameters()->findInt("status")->setIsEnabled(true);
      updater->parameters()->findInt("status")->setValue(static_cast<int>(Status::Pending));
    }
    else if (exitCode != 0)
    {
      updater->parameters()->findInt("state")->setValue(static_cast<int>(State::Completed));
      updater->parameters()->findInt("status")->setIsEnabled(true);
      updater->parameters()->findInt("status")->setValue(static_cast<int>(Status::Failed));
    }
    else if (stage >= static_cast<int>(job->jobType()->stages().size()))
    {
      updater->parameters()->findInt("status")->setIsEnabled(true);
      updater->parameters()->findInt("status")->setValue(static_cast<int>(Status::Succeeded));
    }
    operationManager->launchers()(updater);
  }
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
