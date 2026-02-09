//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/job/Runner.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/operation/JobSpecs.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Observer.h"
#include "smtk/operation/Operation.h"
#include "smtk/task/Task.h"

#include <QPointer>
#include <QProcess>
#include <QVariant>

#include "nlohmann/json.hpp"

namespace smtk
{
namespace qt
{
namespace job
{
class SubmissionSystem
{
public:
  enum JobStatus
  {
    Created,   //!< Job has been added to queue, but is not running.
    Running,   //!< Job is running.
    Completed, //!< Job finished (either it failed or succeeded).
    Cancelled, //!< Job was terminated by the user; results may be incomplete.
    Unknown    //!< Job ID not recognized or invalid.
  };

  using JobIdType = std::uint64_t;

  virtual ~SubmissionSystem() = default;

  static constexpr JobIdType invalidJobId() { return ~std::uint64_t(0); }

  virtual JobIdType queueJob(const std::string& script) = 0;
  virtual bool terminateJob(JobIdType jobId) = 0;
  virtual JobStatus jobStatus(JobIdType jobId) const = 0;
  virtual std::set<JobIdType> jobIds() const = 0;

  // TODO: Add methods to create log parsers which produce streams
  //       that may be read incrementally.
};

class AtQueue : public SubmissionSystem
{
public:
  std::uint64_t queueJob(const std::string& scriptPath) override
  {
    QProcess proc;
    proc.setProgram("at");
    QStringList args;
    args << "now"
         << "-f" << scriptPath.c_str();
    proc.setArguments(args);
    proc.start();
    bool ok;
    auto jobId = static_cast<JobIdType>(proc.readAllStandardOutput().toULongLong(&ok, 10));
    SubmissionSystem::JobStatus jobStatus;
    if (proc.exitStatus() == QProcess::ExitStatus::NormalExit && proc.exitCode() == 0 && ok)
    {
      jobStatus = JobStatus::Running;
      m_jobs[jobId] = jobStatus;
    }
    else
    {
      jobStatus = JobStatus::Unknown;
      jobId = SubmissionSystem::invalidJobId();
    }
    return jobId;
  }

  bool terminateJob(JobIdType jobId) override
  {
    auto it = m_jobs.find(jobId);
    if (it == m_jobs.end() || it->second != JobStatus::Running)
    {
      return false;
    }
    QProcess proc;
    proc.setProgram("at");
    QStringList args;
    args << "-r" << QString::number(jobId);
    proc.setArguments(args);
    proc.start();
    if (proc.exitStatus() == QProcess::ExitStatus::NormalExit && proc.exitCode() == 0)
    {
      it->second = JobStatus::Cancelled;
      return true;
    }
    return false;
  }

  JobStatus jobStatus(JobIdType jobId) const override
  {
    auto it = m_jobs.find(jobId);
    if (it == m_jobs.end())
    {
      return JobStatus::Unknown;
    }
    if (it->second != JobStatus::Running)
    {
      return it->second;
      ;
    }
    it->second = this->updateJobStatus(it);
    // if (it->second == JobStatus::Completed)
    // {
    //   m_jobs.erase(it);
    // }
    return it->second;
  }

  template<typename JobIt>
  JobStatus updateJobStatus(JobIt iterator) const
  {
    QProcess proc;
    proc.setProgram("at");
    QStringList args;
    args << "-q" << QString::number(iterator->first);
    proc.setArguments(args);
    proc.start();
    bool ok;
    auto jobId = static_cast<JobIdType>(proc.readAllStandardOutput().toULongLong(&ok, 10));
    SubmissionSystem::JobStatus jobStatus;
    if (proc.exitStatus() == QProcess::ExitStatus::NormalExit && proc.exitCode() == 0 && ok)
    {
      iterator->second = JobStatus::Running;
    }
    else
    {
      iterator->second = JobStatus::Completed;
    }
    return iterator->second;
  }

  std::set<JobIdType> jobIds() const override
  {
    std::set<JobIdType> jobIds;
    for (auto [jobId, jobStatus] : m_jobs)
    {
      jobIds.insert(jobId);
    }
    return jobIds;
  }

protected:
  mutable std::unordered_map<JobIdType, JobStatus> m_jobs;
};

class ShellQueue : public SubmissionSystem
{
public:
  ~ShellQueue() override
  {
    for (auto [jobId, process] : m_jobs)
    {
      process->kill();
      delete process;
    }
    m_jobs.clear();
  }

  std::uint64_t queueJob(const std::string& scriptPath) override
  {
    QPointer<QProcess> proc = new QProcess;
    proc->setProgram(scriptPath.c_str());
    proc->start();
    SubmissionSystem::JobStatus jobStatus =
      proc->waitForStarted() ? JobStatus::Running : JobStatus::Completed;
    JobIdType jobId = proc->processId();
    if (jobId <= 0)
    {
      jobStatus = JobStatus::Completed;
    }
    else
    {
      proc->setProperty("job-status", static_cast<int>(jobStatus));
      m_jobs[jobId] = proc;
    }
    return jobId;
  }

  bool terminateJob(JobIdType jobId) override
  {
    auto it = m_jobs.find(jobId);
    if (it == m_jobs.end() || it->second->state() == QProcess::NotRunning)
    {
      return false;
    }
    it->second->kill();
    m_jobs.erase(it);
    return true;
  }

  JobStatus jobStatus(JobIdType jobId) const override
  {
    auto it = m_jobs.find(jobId);
    if (it == m_jobs.end())
    {
      return JobStatus::Unknown;
    }
    if (!it->second)
    {
      m_jobs.erase(it);
      return JobStatus::Unknown;
    }
    switch (it->second->state())
    {
      case QProcess::NotRunning:
        return JobStatus::Completed;
      case QProcess::Running:
      case QProcess::Starting:
        return JobStatus::Running;
        break;
      default:
        break;
    }
    return JobStatus::Unknown;
  }

  std::set<JobIdType> jobIds() const override
  {
    std::set<JobIdType> jobIds;
    for (auto [jobId, process] : m_jobs)
    {
      jobIds.insert(jobId);
    }
    return jobIds;
  }

protected:
  mutable std::unordered_map<JobIdType, QPointer<QProcess>> m_jobs;
};

namespace
{

auto g_shellQueue = std::make_shared<ShellQueue>();
auto g_atQueue = std::make_shared<AtQueue>();

} // anonymous namespace

class Runner::Internal
{
public:
  Internal(Runner* self, const std::shared_ptr<smtk::common::Managers>& applicationContext)
    : m_self(self)
    , m_managers(applicationContext)
  {
    if (self && m_managers)
    {
      if (auto opMgr = m_managers->get<smtk::operation::Manager::Ptr>())
      {
        m_operationObserver = opMgr->observers().insert(
          [&](
            const smtk::operation::Operation& op,
            smtk::operation::EventType event,
            smtk::operation::Operation::Result result) -> int {
            if (event == smtk::operation::EventType::DID_OPERATE)
            {
              this->possiblyLaunchJob(op, result);
            }
            return 0; // Never cancel an operation.
          },
          /*priority*/ 0,
          /*initialize*/ false,
          "Observe operations for job-runner.");
      }
    }
  }

  /// Return an object used to queue (and monitor and terminate) job.
  ///
  /// For each job that is queued, a record will be created allowing
  /// the job to be terminated and its status queried. Some submission
  /// systems are robust; others are not.
  std::shared_ptr<SubmissionSystem> findSubmissionSystem(
    smtk::string::Token jobQueueing,
    smtk::string::Token jobLocation)
  {
    using namespace smtk::string::literals;
    // For now, ignore jobLocation. In the future, the jobLocation will
    // modulate what object we return by potentially adding a relay to
    // a pvserver-side object. It may even create a new paraview server
    // connection.
    (void)jobLocation;

    switch (jobQueueing.id())
    {
      default:
        break;
      case "sh"_hash:
        // Just run the job script with a shell.
        return g_shellQueue;
        break;
      case "atd"_hash:
        return g_atQueue;
        break;
        // case "qsub"_hash:
        //   return std::make_shared<QSubQueuer>();
        //   break;
        // Other potential queueing systems to consider:
        // + "podman" (linux/mac/ms)
        // + "docker" (linux/mac/ms)
        // + "schtasks" (ms)
        // + nested queueing (e.g., podman inside a qsub script)
    }
    return std::shared_ptr<SubmissionSystem>();
  }

  void possiblyLaunchJob(
    const smtk::operation::Operation& op,
    smtk::operation::Operation::Result result)
  {
    if (smtk::operation::outcome(result) != smtk::operation::Operation::Outcome::SUCCEEDED)
    {
      return;
    }
#if 0
    if (op.typeName() != m_configuration["monitor-operation"].get<std::string>())
    {
      return;
    }

    auto jobTaskItem = result->findComponent("job-task");
    smtk::task::Task::Ptr jobTask = jobTaskItem && jobTaskItem->isSet() ?
      jobTaskItem->valueAs<smtk::task::Task>() : smtk::task::Task::Ptr();
    if (jobTask.get() != m_self->parent())
    {
      return;
    }
    // Finally we know we should launch a job. Now see if the \a result specifies
    // one and launch it.
    auto scripts = result->findGroup("job-scripts");
    if (scripts) { return; }

    auto& scriptConfig = m_configuration["scripts"];
    scriptConfig = nlohmann::json::array();
#endif
    smtk::operation::visitJobSpecs(
      result,
      "JobSpec",
      [&](
        const smtk::attribute::ReferenceItem::Ptr& tasks,
        const std::filesystem::path& caseDirectory,
        smtk::string::Token jobLocation,
        smtk::string::Token jobQueueing,
        smtk::string::Token jobLauncher,
        const std::vector<std::pair<std::filesystem::path, std::vector<std::filesystem::path>>>&
          scriptLogs) {
        // No task:
        if (!tasks || !tasks->isSet())
        {
          return;
        }

        std::cout << "Case " << caseDirectory << " scripts (" << jobLocation.data() << ", "
                  << jobQueueing.data() << ", " << jobLauncher.data() << " (" << scriptLogs.size()
                  << "))"
                  << "start job"
                  << "\n";
        // Get an object that will queue the job at the given location.
        if (auto submitter = this->findSubmissionSystem(jobQueueing, jobLocation))
        {
          // submitter->queueJob(tasks, caseDirectory, jobLauncher, scriptLogs);
          submitter->queueJob(scriptLogs[0].first);
        }
        else
        {
          smtkErrorMacro(
            smtk::io::Logger::instance(),
            "Could not find submission system \"" << jobQueueing.data()
                                                  << "\" "
                                                     "for location \""
                                                  << jobLocation.data() << "\".");
        }
        // m_configuration["case-directory"] = caseDirectory;
        // m_configuration["job-location"] = jobLocation.data();
        // m_configuration["job-queue-system"] = jobQueueing.data();
        // m_configuration["job-launcher"] = jobLauncher.data();
        for (auto [scriptPath, logPaths] : scriptLogs)
        {
          // scriptConfig.emplace_back<nlohmann::json>({
          //   { "script", scriptPath },
          //   { "log", logPath }});
          std::cout << "  Script " << scriptPath << " with " << logPaths.size() << " logs.\n";
        }
      });
  }

  Runner* m_self{ nullptr };
  QProcess* m_process{ nullptr };
  bool m_lastRunOK{ false };
  std::shared_ptr<smtk::common::Managers> m_managers;
  smtk::operation::Observers::Key m_operationObserver;
};

Runner::Runner(const std::shared_ptr<smtk::common::Managers>& applicationContext)
  : QObject()
  , m_p(std::make_unique<Internal>(this, applicationContext))
{
}

Runner::~Runner()
{
  // TODO: terminate or join all subprocesses, clean up log analyzers.
}

} // namespace job
} // namespace qt
} // namespace smtk
