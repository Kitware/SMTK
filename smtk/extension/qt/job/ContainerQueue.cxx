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
using namespace smtk::string::literals;

class ContainerQueue::Internal
{
public:
  Internal(ContainerQueue* self)
    : m_self(self)
    , m_engine("podman")
  {
  }

  ContainerQueue* m_self{ nullptr };
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
  QProcess proc;
  proc.setProgram(m_p->m_engineExecutable.c_str());
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
      QString userArg = QString("--user=%1:%2").arg(m_p->m_dockerUserId).arg(m_p->m_dockerGroupId);
      processArguments << userArg;
    }
    break;
  }
  std::filesystem::path mountPoint =
    job->caseDirectoryMountPoint().empty() ? "/root" : job->caseDirectoryMountPoint();
  QString volumeArg =
    QString("--volume=%1:%2:z").arg(job->caseDirectory().c_str()).arg(mountPoint.c_str());
  processArguments << volumeArg;

  QString executable = (mountPoint / job->script()).c_str();
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
  proc.start();
  proc.waitForFinished(-1);
  auto queueId = proc.readAllStandardOutput().toStdString();
  job->setQueueId(queueId);
  // TODO: serialize the job to disk so we can check on it even after the GUI has exited+restarted.
  return true;
}

bool ContainerQueue::cancel(const std::shared_ptr<smtk::job::Job>& job)
{
  if (!job || job->queue() != this || job->queueId().empty())
  {
    return false;
  }
  QProcess proc;
  proc.setProgram(m_p->m_engineExecutable.c_str());
  QStringList processArguments;
  processArguments << "kill" << QString::fromStdString(job->queueId());

  proc.start();
  if (!proc.waitForFinished())
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Could not run kill command.");
    return false;
  }
  // The command completed. If successful, assume the container has been destroyed.
  if (proc.exitStatus() != QProcess::ExitStatus::NormalExit || proc.exitCode() != 0)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Kill command failed.");
    return false;
  }
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

smtk::string::Token ContainerQueue::engine() const
{
  return m_p->m_engine;
}

std::filesystem::path ContainerQueue::engineExecutable() const
{
  return m_p->m_engineExecutable;
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

} // namespace job
} // namespace qt
} // namespace smtk
