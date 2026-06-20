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

class ContainerQueue::Internal
{
public:
  Internal(ContainerQueue* self)
    : m_self(self)
    , m_watcher(new QFileSystemWatcher)
    , m_engine("podman")
  {
    QObject::connect(
      m_watcher.data(), &QFileSystemWatcher::fileChanged, self, &ContainerQueue::fileUpdated);
    QObject::connect(
      m_watcher.data(),
      &QFileSystemWatcher::directoryChanged,
      self,
      &ContainerQueue::directoryUpdated);
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

  ContainerQueue* m_self{ nullptr };
  QScopedPointer<QFileSystemWatcher> m_watcher;
  std::unordered_map<QString, std::function<void(const QString&)>> m_dispatch;
  smtk::string::Token m_engine;

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

ContainerQueue::~ContainerQueue() {}

bool ContainerQueue::schedule(const std::shared_ptr<smtk::job::Job>& job)
{
  if (!job || !job->queueId().empty() || job->containerImage().empty())
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
  proc.setProgram(QString::fromStdString(this->engineExecutable().string()));
  proc.setWorkingDirectory(QString::fromStdString(job->caseDirectory().string()));
  // We watch files for updates on the host OS, not the container OS.
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

  QString executable = QString::fromStdString((mountPoint / job->script()).string());
  processArguments << job->containerImage().c_str() << executable;

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
  job->setStage(-1);
  job->setStatus(smtk::job::Status::Pending);
  proc.start();
  proc.waitForFinished(-1);
  auto queueId = proc.readAllStandardOutput().toStdString();
  job->setQueueId(queueId);
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

bool ContainerQueue::setCaseDirectoryMountPoint(const std::filesystem::path& mountPoint)
{
  auto prev = m_caseDirectoryMountPoint;
  if (prev == mountPoint)
  {
    return false;
  }
  m_caseDirectoryMountPoint = mountPoint;
  this->projectRootChanged(mountPoint);
  Q_EMIT caseDirectoryMountPointChanging(prev, m_caseDirectoryMountPoint);
  return true;
}

QString ContainerQueue::caseDirectoryMountPointAsString()
{
  return QString::fromStdString(m_caseDirectoryMountPoint.string());
}

bool ContainerQueue::setCaseDirectoryMountPointAsString(const QString& mountPoint)
{
  std::filesystem::path path = mountPoint.toStdString();
  return this->setCaseDirectoryMountPoint(path);
}

void ContainerQueue::projectRootChanged(const std::filesystem::path& nextProjectRoot)
{
  // TODO: Restart podman machine (on Windows and MacOS only) with a new "-v" option
  //       mapping ProjectsRootFolder into the machine.
  //       This allows containers running on the machine to mount case directories.
  std::cerr << "TODO: Update podman machine\n";
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

void ContainerQueue::fileUpdated(const QString& path)
{
  // std::cerr << "File Path \"" << path.toStdString() << "\" updated.\n";
  m_p->dispatchUpdate(path);
}

void ContainerQueue::directoryUpdated(const QString& path)
{
  // std::cerr << "Directory Path \"" << path.toStdString() << "\" updated.\n";
  m_p->dispatchUpdate(path);
}

} // namespace job
} // namespace qt
} // namespace smtk
