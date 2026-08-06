//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/qt/job/ContainerQueue.h"

#include "smtk/extension/qt/job/UpdateContainerQueueMachine.h"

#include "smtk/job/Definition.h"
#include "smtk/job/Job.h"
#include "smtk/job/Queue.h"
#include "smtk/job/db/BindingText.h"
#include "smtk/job/db/Query.h"
#include "smtk/job/operators/JobUpdated.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"

#include "smtk/common/CompilerInformation.h"

#include <QPointer>
#include <QProcess>
#include <QTimer>

#include <algorithm>
#include <cctype>
#include <cstdlib> // For std::system
#include <fstream>
#include <string>

// Note that SMTK_PLATFORM_UNIX should only be set on non-macos
// unix platforms as podman on macos requires a virtual machine.
#if !defined(_WIN32) && !defined(__CYGWIN__)
#ifdef __APPLE__
#define SMTK_PLATFORM_UNIX 0
#else
#define SMTK_PLATFORM_UNIX 1
#endif
#else
#define SMTK_PLATFORM_UNIX 0
#endif

#if SMTK_PLATFORM_UNIX
#include <unistd.h> // For sysconf()
#endif

using namespace smtk::job::db;

namespace smtk
{
namespace qt
{
namespace job
{
namespace
{

// Trim from the start (in place)
inline std::string& ltrim(std::string& s)
{
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
          }));
  return s;
}

// Trim from the end (in place)
inline std::string& rtrim(std::string& s)
{
  s.erase(
    std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(),
    s.end());
  return s;
}

// Trim both ends (in place)
inline std::string& trim(std::string& s)
{
  ltrim(s);
  rtrim(s);
  return s;
}

} // anonymous namespace

using namespace smtk::job;
using namespace smtk::string::literals;

class ContainerQueue::Internal
{
public:
  Internal(ContainerQueue* self)
    : m_self(self)
    , m_engine("podman")
  {
    // Every 100ms, poll any case-directory progress files that may have changed.
    m_timer.setInterval(100 /*ms*/);
    m_timer.setSingleShot(false);
    QObject::connect(&m_timer, &QTimer::timeout, self, &ContainerQueue::updateJobStates);
    m_timer.start();
    // QObject::connect(m_watcher.data(), &QFileSystemWatcher::fileChanged,
    //   self, &ContainerQueue::fileUpdated);
    // QObject::connect(m_watcher.data(), &QFileSystemWatcher::directoryChanged,
    //   self, &ContainerQueue::directoryUpdated);
  }

  using PathUpdateResponder = std::function<void(const QString&)>;

  void startPollingJobProgress(Job* job)
  {
    if (!job)
    {
      return;
    }
    std::lock_guard<std::mutex> lock(m_pollLock);
    m_pathsToPoll[job->caseDirectory() / "logs" / "progress"] = job->id();
  }

  void stopPollingJobProgress(Job* job)
  {
    if (!job)
    {
      return;
    }

    std::lock_guard<std::mutex> lock(m_pollLock);
    m_pathsToPoll.erase(job->caseDirectory() / "logs" / "progress");
  }

  void updateJobStates()
  {
    auto operationManager = m_self->operationManager();
    if (!operationManager)
    {
      return;
    }

    std::unordered_map<std::filesystem::path, smtk::common::UUID> pathMap;
    {
      std::lock_guard<std::mutex> lock(m_pollLock);
      pathMap = m_pathsToPoll;
    }
    for (auto [path, jobId] : pathMap)
    {
      std::ifstream pp(path);
      int stage = -3;
      pp >> stage;
      if (pp.good() && stage > -3)
      {
        // std::cerr << "  Stage " << stage << "\n";
        if (auto job = m_self->findJob(jobId))
        {
          if (
            job->stage() == stage || job->state() < smtk::job::Scheduled ||
            job->state() > smtk::job::Running)
          {
            // Do not fire an update when not needed.
            continue;
          }
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
            // No need to watch this job any longer.
            {
              std::lock_guard<std::mutex> lock(m_pollLock);
              m_pathsToPoll.erase(path);
            }
          }
          operationManager->launchers()(updater);
        }
        else
        {
          // Job was removed from Queue?
          smtkErrorMacro(
            smtk::io::Logger::instance(),
            "Job " << jobId << " has been updated but is not present in queue.");
        }
      }
    }
  }

  /// The parent queue which owns this Internal object.
  ContainerQueue* m_self{ nullptr };
  /// The container engine (podman or docker) used to run jobs.
  smtk::string::Token m_engine;
  /// A timer for polling "active" jobs for updates.
  QTimer m_timer;
  /// A mutex used to lock access to m_pathsToPoll. This is needed since the GUI
  /// thread will be polling jobs while operations will be adding/removing/updating them.
  std::mutex m_pollLock;
  /// The active job set. A map from absolute paths to "logs/progress" files to job UUID.
  /// Each time m_timer fires, all of these paths are polled to see if the job's stage
  /// can be updated.
  std::unordered_map<std::filesystem::path, smtk::common::UUID> m_pathsToPoll;

  /// The user ID to use when running processes in a docker container.
  int m_dockerUserId{ -1 };
  /// The group ID to use when running processes in a docker container.
  int m_dockerGroupId{ -1 };
  /// The path to the executable for m_engine.
  std::filesystem::path m_engineExecutable;
};

// ContainerQueue::ContainerQueue(const std::shared_ptr<smtk::common::Managers>& applicationContext);
ContainerQueue::ContainerQueue()
  : m_p(new ContainerQueue::Internal(this))
{
  // TODO: Deserialize local jobs from storage?
}

ContainerQueue::ContainerQueue(
  const smtk::common::UUID& uid,
  const std::shared_ptr<smtk::resource::Manager>& resourceManager)
  : Superclass(uid, resourceManager)
  , m_p(new ContainerQueue::Internal(this))
{
}

ContainerQueue::~ContainerQueue()
{
  bool containersInVM = true;
#if SMTK_PLATFORM_UNIX
  // On linux, only use a virtual machine if forced to by an environment variable.
  auto* forceVM = std::getenv("SMTK_PODMAN_MACHINE");
  if (!forceVM || !forceVM[0])
  {
    containersInVM = false;
  }
#endif
  if (m_removeQueueOnDestruction && containersInVM)
  {
    // std::cerr << "Destroying virtual machine for " << this->name() << "\n";
    {
      QProcess proc;
      QStringList processArguments;
      proc.setProgram(QString::fromStdString(this->engineExecutable().string()));
      processArguments << "machine"
                       << "stop" << QString::fromStdString(this->name());
      proc.setArguments(processArguments);
      proc.start();
      proc.waitForFinished(-1);
      // std::cerr << proc.readAllStandardOutput().toStdString() << "\n";
      // std::cerr << proc.readAllStandardError().toStdString() << "\n";
    }
    {
      QProcess proc;
      QStringList processArguments;
      proc.setProgram(QString::fromStdString(this->engineExecutable().string()));
      processArguments << "machine"
                       << "rm"
                       << "-f" << QString::fromStdString(this->name());
      proc.setArguments(processArguments);
      proc.start();
      proc.waitForFinished(-1);
      // std::cerr << proc.readAllStandardOutput().toStdString() << "\n";
      // std::cerr << proc.readAllStandardError().toStdString() << "\n";
    }
  }
}

bool ContainerQueue::schedule(const std::shared_ptr<smtk::job::Job>& job)
{
  if (!job || !job->queueId().empty())
  {
    return false;
  }
  if (job->containerImage().empty())
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Cannot schedule job; no container image provided.");
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
  proc.setProgram(QString::fromStdString(this->engineExecutable().string()));
  proc.setWorkingDirectory(QString::fromStdString(job->caseDirectory().string()));
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
  QStringList processArguments;
  processArguments << "run"
                   << "-d";
  switch (m_p->m_engine.id())
  {
    case "podman"_hash:
      processArguments << "--userns=keep-id";
      break;
    case "docker"_hash:
    {
      if (m_p->m_dockerUserId >= 0 && m_p->m_dockerGroupId >= 0)
      {
        QString userArg =
          QString("--user=%1:%2").arg(m_p->m_dockerUserId).arg(m_p->m_dockerGroupId);
        processArguments << userArg;
      }
    }
    break;
  }
  std::filesystem::path mountPoint =
    job->caseDirectoryMountPoint().empty() ? "/root" : job->caseDirectoryMountPoint();
  QString volumeArg =
    QString("--volume=%1:%2:z").arg(job->caseDirectory().c_str()).arg(mountPoint.c_str());
  processArguments << volumeArg;
  // Do not leave container lying around on podman desktop; once it exits,
  // remove it:
  processArguments << "--rm";

  // QString executable = "./" + QString::fromStdString(job->script().string());
  // Containers are Linux and do not like windows path separators; so use generic_string():
  QString executable = QString::fromStdString((mountPoint / job->script()).generic_string());
  processArguments << job->containerImage().c_str() << executable;

  proc.setArguments(processArguments);
#if 0
  std::cerr << "running \"" << proc.program().toStdString();
  for (const auto& arg : proc.arguments())
  {
    std::cerr << " " << arg.toStdString();
  }
  std::cerr << "\"\n";
#endif // 0
  // Set the job stage and status:
  job->setStage(-1);
  job->setState(smtk::job::State::Scheduled);
  job->setStatus(smtk::job::Status::Pending);
  m_p->startPollingJobProgress(job.get());
  // Because launching a container with "-d" (--detach) returns immediately, wait
  // until we get the container ID as output; it will serve as the job's queue ID.
  proc.start();
  proc.waitForFinished(-1);
#if 0
  std::cerr
    << "  exit status " << static_cast<int>(proc.exitStatus())
    << " code " << proc.exitCode() << "\n";
#endif // 0
  auto queueId = proc.readAllStandardOutput().toStdString();
  std::cerr << "     job id " << queueId << "\n";
  // Trim the queue ID to eliminate newlines from stdout.
  // When podman/docker are run with "-d", they print the container's UUID to stdout.
  job->setQueueId(trim(queueId));
  // Serialize to the database.
  this->updateJobDatabaseInfo(job);
  return true;
}

bool ContainerQueue::cancel(const std::shared_ptr<smtk::job::Job>& job)
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
  QProcess proc;
  proc.setProgram(QString::fromStdString(this->engineExecutable().string()));
  QStringList processArguments;
  processArguments << "container"
                   << "kill" << QString::fromStdString(job->queueId().substr(0, 12));
  proc.setArguments(processArguments);
#if 0
  std::cerr << "running \"" << proc.program().toStdString();
  for (const auto& arg : proc.arguments())
  {
    std::cerr << " " << arg.toStdString();
  }
  std::cerr << "\"\n";
#endif

  proc.start();
  if (!proc.waitForFinished())
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Could not run kill command.");
    return false;
  }
  // The command completed. If successful, assume the container has been destroyed.
  if (proc.exitStatus() != QProcess::ExitStatus::NormalExit || proc.exitCode() != 0)
  {
    // For debugging:
    // smtkErrorMacro(smtk::io::Logger::instance(),
    //   "Kill command failed (status " << proc.exitStatus() << " code " << proc.exitCode() << ").");
    return false;
  }
  // Lock the map of paths to poll and remove this job's progress path.
  // After this, we can guarantee no more operations will be queued from
  // updateJobStates(), though some jobs updating the job may already
  // have been submitted. They should not update the job state, though,
  // once it is set to canceled/terminated.
  m_p->stopPollingJobProgress(job.get());
  job->setState(State::Canceled);
  job->setStatus(Status::Terminated);
  this->updateJobDatabaseInfo(job);
  // TODO: serialize to disk
  return true;
}

State ContainerQueue::jobState(const std::shared_ptr<smtk::job::Job>& job)
{
  if (!job || job->queue() != this || job->queueId().empty())
  {
    return State::Unscheduled;
  }
  // TODO: Run "ps" to get state of job->queueId().
  return State::Completed;
}

Status ContainerQueue::jobStatus(const std::shared_ptr<smtk::job::Job>& job)
{
  if (!job || job->queue() != this)
  {
    return Status::Pending;
  }
  return job->queueId().empty() ? Status::Succeeded : Status::Pending;
}

std::set<std::shared_ptr<smtk::job::Job>> ContainerQueue::allJobs() const
{
  std::set<std::shared_ptr<smtk::job::Job>> result;
  return result;
}

bool ContainerQueue::pullContainerImage(const std::string& imageUrl)
{
  QProcess proc;
  proc.setProgram(QString::fromStdString(this->engineExecutable().string()));
  QStringList processArguments;
  processArguments << "pull" << QString::fromStdString(imageUrl);
  proc.setArguments(processArguments);
#if 0
  std::cerr << "running \"" << proc.program().toStdString();
  for (const auto& arg : proc.arguments())
  {
    std::cerr << " " << arg.toStdString();
  }
  std::cerr << "\"\n";
#endif
  // Because launching a container with "-d" (--detach) returns immediately, wait
  // until we get the container ID as output; it will serve as the job's queue ID.
  proc.start();
  proc.waitForFinished(-1);
  return (proc.exitStatus() == QProcess::ExitStatus::NormalExit && proc.exitCode() == 0);
}

bool ContainerQueue::setRootJobDirectory(const std::filesystem::path& mountPoint)
{
  auto prev = m_rootJobDirectory;
  if (prev == mountPoint)
  {
    return false;
  }
  m_rootJobDirectory = mountPoint;
  this->projectRootChanged(mountPoint);
  Q_EMIT rootJobDirectoryChanging(prev, m_rootJobDirectory);
  return true;
}

QString ContainerQueue::rootJobDirectoryAsString()
{
  return QString::fromStdString(m_rootJobDirectory.string());
}

bool ContainerQueue::setRootJobDirectoryAsString(const QString& mountPoint)
{
  std::filesystem::path path = mountPoint.toStdString();
  return this->setRootJobDirectory(path);
}

void ContainerQueue::projectRootChanged(const std::filesystem::path& nextProjectRoot)
{
#if SMTK_PLATFORM_UNIX
  // On linux, only use a virtual machine if forced to by an environment variable.
  auto* forceVM = std::getenv("SMTK_PODMAN_MACHINE");
  if (!forceVM || !forceVM[0])
  {
    return;
  }
#endif
  // Restart podman machine (on Windows and MacOS only) with a new "-v" option
  // mapping ProjectsRootFolder into the machine.
  // This allows containers running on the machine to mount case directories.
  bool didLaunch = false;
  if (auto operationManager = this->operationManager())
  {
    auto op = operationManager->create<UpdateContainerQueueMachine>();
    if (op->parameters()->associate(shared_from_this()))
    {
      operationManager->launchers()(op);
      didLaunch = true;
    }
  }
  if (!didLaunch)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Could not launch a job to reset " << this->name() << "'s machine.");
  }
}

smtk::string::Token ContainerQueue::engine() const
{
  return m_p->m_engine;
}

std::filesystem::path ContainerQueue::engineExecutable() const
{
  return m_p->m_engineExecutable.empty() ? std::filesystem::path(m_p->m_engine.data())
                                         : m_p->m_engineExecutable;
}

bool ContainerQueue::setEngineExecutable(std::filesystem::path engineExecutable)
{
  if (engineExecutable == m_p->m_engineExecutable)
  {
    return false;
  }
  if (engineExecutable.string().rfind("podman") != std::string::npos)
  {
    m_p->m_engine = "podman"_token;
    m_p->m_engineExecutable = engineExecutable;
    return true;
  }
  else if (engineExecutable.string().rfind("docker") != std::string::npos)
  {
    m_p->m_engine = "docker"_token;
    m_p->m_engineExecutable = engineExecutable;
    return true;
  }
  return false;
}

int ContainerQueue::dockerUID() const
{
  return m_p->m_dockerUserId;
}

bool ContainerQueue::setDockerUID(int uid)
{
  if (uid == m_p->m_dockerUserId || uid < 0)
  {
    return false;
  }
  m_p->m_dockerUserId = uid;
  return true;
}

int ContainerQueue::dockerGID() const
{
  return m_p->m_dockerGroupId;
}

bool ContainerQueue::setDockerGID(int gid)
{
  if (gid == m_p->m_dockerGroupId || gid < 0)
  {
    return false;
  }
  m_p->m_dockerGroupId = gid;
  return true;
}

bool ContainerQueue::setQueueOnline(bool online)
{
  if (m_queueOnline == online)
  {
    return false;
  }
  m_queueOnline = online;
  return true;
}

void ContainerQueue::updateJobStates()
{
  m_p->updateJobStates();
}

bool ContainerQueue::checkQueueRoot(const std::filesystem::path& root)
{
#if SMTK_PLATFORM_UNIX
  // On linux, only use a virtual machine if forced to by an environment variable.
  auto* forceVM = std::getenv("SMTK_PODMAN_MACHINE");
  if (!forceVM || !forceVM[0])
  {
    // If we don't need to start (or stop, delete, init, and start) the machine,
    // we do want to limit the maximum queue size if the registrar set it to a
    // negative number.
    int maxJobSize = 0;
#ifdef _SC_NPROCESSORS_ONLN
    maxJobSize = static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN));
#endif
    if (maxJobSize > 0)
    {
      // Containers should use at most half the processors
      maxJobSize = (maxJobSize > 1 ? maxJobSize / 2 : 1);
      // This call will only update the maximum size if
      // the registrar initialized the m_maximumSize ivar to -1.
      // This allows queues to have limits unrelated to the actual
      // core count of a machine.
      this->setMaximumJobSize(maxJobSize);
    }
    return false;
  }
#endif
  std::string directory;
  sqlQuery query(m_db);
  query << "select value from queue_metadata where queue=" << m_queueId
        << " and key='root_job_directory';";
  query.bind<sqlBindingText<std::string>>(0, directory);
  if (!query.execute() || directory.empty())
  {
    std::cerr << "checkQueueRoot indicates update required.\n";
    return false;
  }
  std::cerr << "checkQueueRoot indicates update required? " << (directory == root.string()) << "\n";
  return (directory == root.string());
}

bool ContainerQueue::setMetadata(const std::string& key, const std::string& value)
{
  sqlQuery query(m_db);
  query << "insert into queue_metadata (queue, key, value) values (" << m_queueId << ",'" << key
        << "','" << value << "');";
  return query.execute();
}

} // namespace job
} // namespace qt
} // namespace smtk
