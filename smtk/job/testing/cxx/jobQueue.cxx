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
#include "smtk/extension/qt/job/ShellQueue.h"
#include "smtk/extension/qt/qtViewRegistrar.h"

#include "smtk/job/DatabaseQueue.h"
#include "smtk/job/Definition.h"
#include "smtk/job/Job.h"
#include "smtk/job/Manager.h"
#include "smtk/job/Registrar.h"
#include "smtk/job/Stage.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"
#include "smtk/operation/Registrar.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"

#include "smtk/io/Logger.h"

#include "smtk/plugin/Registry.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Registrar.h"

#include "smtk/common/Managers.h"
#include "smtk/common/Paths.h"
#include "smtk/common/UUID.h"

#include "smtk/string/Token.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QObject>
#include <QThread>
#include <QTimer>
#include <QVariant>

#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <thread>

using namespace smtk::string::literals;
using namespace smtk::job;

namespace // anonymous
{

// The "job" to perform.
// TODO: This should test that an input file exists and generate
//       an output file from it (in addition to "logs/job.log").
// TODO: This should test concurrency with mpiexec (or srun, etc.).
std::string job_script_text = R"foo(#!/bin/bash
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd $SCRIPT_DIR
# echo TestJob: starting
echo "0 0" > logs/progress
echo "not yet" > logs/job.log
# echo TestJob: Progress initialized
sleep 0.00625
echo "wait for it" >> logs/job.log
echo "1 0" > logs/progress
# echo TestJob: Stage 1 complete
echo "done" >> logs/job.log
sleep 0.00625
# echo TestJob: Stage 2 complete
echo "2 0" > logs/progress
)foo";

std::string job_script_cancel_text = R"foo(#!/bin/bash
# echo TestJob: starting
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd $SCRIPT_DIR
echo "0 0" > logs/progress
echo "not yet" > logs/job.log
# echo TestJob: Progress initialized
sleep 2
echo "wait for it" >> logs/job.log
echo "1 0" > logs/progress
# echo TestJob: Stage 1 complete
echo "done" >> logs/job.log
sleep 20
# echo TestJob: Stage 2 complete
echo "2 0" > logs/progress
)foo";

smtk::common::UUID g_jobId;
bool g_expectToCancel{ false };
std::set<std::filesystem::path> g_caseDirectories;

class JobCreatorOp : public smtk::operation::Operation
{
public:
  smtkTypeMacro(JobCreatorOp);
  smtkCreateMacro(JobCreatorOp);
  smtkSuperclassMacro(smtk::operation::Operation);
  smtkSharedFromThisMacro(smtk::operation::Operation);

  bool
  writeFile(std::filesystem::path dir, const std::filesystem::path path, const std::string& data)
  {
    auto filePath = dir / path;
    try
    {
      std::ofstream file(filePath.c_str());
      file << data;
      file.close();
      std::filesystem::permissions(
        filePath,
        std::filesystem::perms::owner_read | std::filesystem::perms::group_read |
          std::filesystem::perms::owner_exec | std::filesystem::perms::group_exec,
        std::filesystem::perm_options::add);
    }
    catch (std::exception& e)
    {
      std::cerr << "ERROR: Could not create file \"" << filePath.c_str() << "\" (" << e.what()
                << ").\n";
      return false;
    }
    return true;
  }

  Result operateInternal() override
  {
    auto job = smtk::job::Job::create();
    auto jobManager = this->managers()->get<smtk::job::Manager::Ptr>();
    if (!jobManager)
    {
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }
    auto jobDef = jobManager->jobTypes().findByName("Test");
    if (!jobDef)
    {
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }
    job->setJobType(jobDef);
    // ContainerQueue mounts SMTK_SCRATCH_DIR into its VM. Create cases under
    // that same host directory so both shell and container jobs can access them.
    const auto tempDir = std::filesystem::path(SMTK_SCRATCH_DIR);
    std::filesystem::create_directories(tempDir);
    std::string pattern = (tempDir / "testQueueJob_XXXXXX").string();
    auto caseDir = generateDirectory(pattern);
    std::filesystem::create_directories(caseDir / "logs");
    std::string testType = this->parameters()->findString("test type")->value();
    if (testType == "basic")
    {
      this->writeFile(caseDir, jobDef->script(), job_script_text);
    }
    else if (testType == "cancel")
    {
      // This adds a longer pause to allow job cancellation to be tested:
      this->writeFile(caseDir, jobDef->script(), job_script_cancel_text);
    }
    else
    {
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }
    this->writeFile(caseDir, "logs/progress", "-1");
    std::cerr << "Case \"" << caseDir.string() << "\"\n";

    job->setId(g_jobId);
    job->setSize(1);            // Don't run in parallel
    job->setAutoSchedule(true); // Schedule job as soon as added to the queue.
    if (auto jobManager = this->managers()->get<smtk::job::Manager::Ptr>())
    {
      job->setQueue(jobManager->activeQueue().object());
    }
    else
    {
      std::cerr << "ERROR: No job manager (needed for active queue).\n";
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }
    job->setCaseDirectory(caseDir);
    if (job->queue()->hasTag("container"_token))
    {
      job->setContainerImage("ubuntu:26.04");
      job->setCaseDirectoryMountPoint("/testing");
    }
    std::cout << "Created job " << job->id() << "\n";

    auto result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
    auto jobsItem = result->findComponent("created");
    jobsItem->appendValue(job);
    return result;
  }

  Specification createSpecification() override
  {
    auto spec = this->createBaseSpecification();
    auto pdef = spec->createDefinition("JobCreatorOp", "operation");
    auto ttyp = smtk::attribute::StringItemDefinition::New("test type");
    pdef->addItemDefinition(ttyp);
    spec->createDefinition("result(JobCreatorOp)", "result");
    return spec;
  }
};

/// The meat of the test for queueing a job.
///
/// This is in a QObject run from a timer inside the main thread's event loop.
/// It waits until the queued job completes before causing the event loop to
/// terminate. There is also a "dead man" timer to force an exit in case the
/// job fails to reach completion.
class JobQueueTest : public QObject
{
public:
  JobQueueTest(QCoreApplication* app)
    : m_app(app)
    , m_appContext(smtk::common::Managers::create())
    , m_resourceRegistry(smtk::plugin::addToManagers<smtk::resource::Registrar>(m_appContext))
    , m_operationRegistry(smtk::plugin::addToManagers<smtk::operation::Registrar>(m_appContext))
    , m_resourceManager(m_appContext->get<smtk::resource::Manager::Ptr>())
    , m_operationManager(m_appContext->get<smtk::operation::Manager::Ptr>())
    , m_jobRegistry(smtk::plugin::addToManagers<smtk::job::Registrar>(
        m_appContext,
        m_resourceManager,
        m_operationManager))
    , m_jobManager(m_appContext->get<smtk::job::Manager::Ptr>())
    , m_qtRegistry(smtk::plugin::addToManagers<smtk::extension::qtViewRegistrar>(
        m_appContext,
        m_operationManager,
        m_resourceManager,
        m_jobManager))
  {
    m_timer.reset(new QTimer);
    m_timer->setInterval(100);
    m_timer->setSingleShot(true);
    QObject::connect(m_timer.data(), &QTimer::timeout, m_app, &QCoreApplication::quit);

    // Create a new type of job and register it.
    auto jobType = smtk::job::Definition::create();
    std::filesystem::path scriptPath = "run_job.sh";
    std::filesystem::path logPath = "logs/job.log";
    jobType->setName("Test");
    jobType->setScript(scriptPath);
    // We have two stages:
    auto stage = jobType->appendStage("Pretending", "Pretend to do work", logPath);
    stage = jobType->appendStage("Hallucinating", "Hallucinate results", logPath);
    stage->addArtifact("result.data");
    m_jobManager->jobTypes().manage(jobType);

    // Register our test operation whose result includes a job to queue:
    m_operationManager->registerOperation<JobCreatorOp>();

    // Create a temporary queue for local jobs.
    m_shellQueue = smtk::qt::job::ShellQueue::createOrRestore<smtk::qt::job::ShellQueue>(
      "bar",
      "A queue full of foo",
      "localhost",
      /* maximum job size */ 8,
      /* capability tags */ { "shell"_token, "bar"_token, "bash"_token },
      /* remove queue on destruction */ true,
      smtk::common::UUID("ad173a6f-1f28-4c34-b476-a08623befecd"),
      m_resourceManager,
      m_operationManager,
      m_jobManager);
    // Exercise the interpreter launch path used by Windows Bash scripts. The test
    // scripts are Bash scripts, so this is equivalent to direct execution on
    // Unix while covering argument insertion and relative script handling.
    m_shellQueue->setInterpreter("/bin/bash");
    m_shellQueue->setInterpreterArguments({ "--noprofile", "--norc" });
    std::shared_ptr<smtk::job::Queue> queue = m_shellQueue;
    std::cout << "Queue " << queue->name() << " is a " << queue->typeName() << "\n";

    m_containerQueue =
      smtk::qt::job::ContainerQueue::createOrRestore<smtk::qt::job::ContainerQueue>(
        "baz",
        "A test container queue.",
        "localhost",
        /* maximum job size */ 8,
        /* capability tags */ { "container"_token, "baz"_token, "podman"_token },
        /* container engine executable */ "podman",
        /* remove queue on destruction */ true,
        smtk::common::UUID("9a63e63d-5eeb-4230-8404-c1d32deb6ce8"),
        /* root job directory */ SMTK_SCRATCH_DIR,
        /* docker UID */ -1,
        /* docker GID */ -1,
        m_resourceManager,
        m_operationManager,
        m_jobManager);
    queue = m_containerQueue;
    std::cout << "Queue " << queue->name() << " is a " << queue->typeName() << "\n";
    // Update the VM manually (unlike the paraview::job::Registrar, which monitors changes
    // to the root_job_directory and recreates the VM as needed).
    auto op = m_operationManager->create("smtk::qt::job::UpdateContainerQueueMachine");
    // A previous aborted run may leave queue metadata on disk. Explicitly
    // initialize the runtime mount path even when that metadata already matches.
    m_containerQueue->setProperty("rootJobDirectory", QString(SMTK_SCRATCH_DIR));
    op->parameters()->associate(m_containerQueue);
    auto setupResult = op->operate();
    if (
      setupResult->findInt("outcome")->value() !=
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED))
    {
      std::cerr << "ERROR: Could not initialize the test container queue.\n";
      return;
    }
    // Pull an image to run.
    std::cout << "  Pulling image ubuntu:26.04\n";
    if (!m_containerQueue->pullContainerImage("ubuntu:26.04"))
    {
      std::cerr << "ERROR: Could not pull the test container image.\n";
      return;
    }
    std::cout << "    done\n";

    m_jobManager->queues().manage(m_shellQueue);
    m_jobManager->queues().manage(m_containerQueue);
    m_jobManager->activeQueue().switchTo(m_shellQueue.get());

    // Add a job observer to update JobQueueTest with a result
    // as AddJobToQueue and JobUpdated are invoked in response to JobCreatorOp.
    m_shellQueueUpdateKey = m_shellQueue->observe(nullptr, this->jobStateObserver(), false);
    m_containerQueueUpdateKey = m_containerQueue->observe(nullptr, this->jobStateObserver(), false);
    m_ready = true;
  }

  std::function<void(const smtk::job::Job& job)> jobStateObserver()
  {
    return [this](const smtk::job::Job& job) {
      std::string jobName = job.name();
      if (jobName.size() > 12)
      {
        jobName = jobName.substr(0, 12);
      }
      std::cout << "  Job " << jobName << " stage " << std::setw(2) << job.stage() << " "
                << std::setw(12) << smtk::job::stateAsString(job.state()) << " " << std::setw(12)
                << smtk::job::statusAsString(job.status());
      if (job.stage() < 0)
      {
        std::cout << " (starting)";
      }
      else if (job.stage() < job.jobType()->stages().size())
      {
        std::cout << " \"" << job.jobType()->stages()[job.stage()]->name() << "\"";
      }
      else
      {
        std::cout << " (completed)";
      }
      std::cout << "\n";

      // Queue-wide observers also receive late updates from previous jobs.
      // Snapshot the notification on the operation thread, then handle phase
      // state on the Qt thread. Only the current job may finish this phase,
      // and Canceled -> Completed notifications must not finish it twice.
      const auto id = job.id();
      const auto state = job.state();
      const bool finished = job.stage() == job.jobType()->stages().size() ||
        job.status() == smtk::job::Status::Terminated;
      const auto caseDirectory = job.caseDirectory();
      QMetaObject::invokeMethod(
        this,
        [this, id, state, finished, caseDirectory]() {
          if (id != g_jobId || m_phaseFinished)
          {
            return;
          }
          // Wait for scheduling instead of assuming that VM/container startup
          // completes within a fixed delay. Request cancellation only once.
          if (
            g_expectToCancel && !m_cancelRequested &&
            (state == State::Scheduled || state == State::Running))
          {
            m_cancelRequested = true;
            this->cancel_job();
          }
          if (!finished)
          {
            return;
          }
          m_phaseFinished = true;
          g_caseDirectories.insert(caseDirectory);
          if (
            (g_expectToCancel && state == State::Canceled) ||
            (!g_expectToCancel && state == State::Completed))
          {
            m_result = 0;
          }
          else
          {
            std::cerr << "ERROR: Unexpected end state " << smtk::job::stateAsString(state) << "\n";
            m_result = 56;
          }
          m_timer->start();
        },
        Qt::QueuedConnection);
    };
  }
  void reset()
  {
    m_result = 255;
    m_phaseFinished = false;
    m_cancelRequested = false;
    m_timer->stop();
  }

public Q_SLOTS:

  void test_basic()
  {
    g_jobId = smtk::common::UUID::random();
    g_expectToCancel = false;
    auto jobOp = m_operationManager->create<JobCreatorOp>();
    jobOp->parameters()->findString("test type")->setValue("basic");
    m_operationManager->launchers()(jobOp);
    // Now if jobOp succeeds, more operations will follow (AddJobToQueue, then JobUpdated
    // once for each time the job stage modification is detected). After the final stage, our
    // job observer will signal Qt to stop the event loop.
  }

  void test_cancel()
  {
    g_jobId = smtk::common::UUID::random();
    g_expectToCancel = true;
    auto jobOp = m_operationManager->create<JobCreatorOp>();
    jobOp->parameters()->findString("test type")->setValue("cancel");
    m_operationManager->launchers()(jobOp);
    // Now if jobOp succeeds, more operations will follow (AddJobToQueue, then JobUpdated
    // once for each time the job stage modification is detected). After the final stage, our
    // job observer will signal Qt to stop the event loop.
  }

  void cancel_job()
  {
    auto queue = m_jobManager->activeQueue().object();
    auto job = queue->findJob(g_jobId);
    for (int ii = 0; !job && ii < 20; ++ii)
    {
      using namespace std::chrono_literals;
      std::this_thread::sleep_for(100ms);
      job = queue->findJob(g_jobId);
    }
    if (!job)
    {
      std::cerr << "ERROR: Could not find job " << g_jobId << " to cancel in queue \""
                << queue->name() << "\".\n";
      m_result = 42;
      return;
    }
    auto cancelOp = m_operationManager->create("smtk::job::CancelJob");
    cancelOp->parameters()->associate(job);
    m_operationManager->launchers()(cancelOp);
    // std::cerr << "Launch op " << cancelOp << " to cancel " << g_jobId << "\n";
  }

  // Verify rejection after completion explicitly; a delayed cancellation timer
  // can otherwise fire in a later phase, or never run before this phase exits.
  int test_cancel_completed()
  {
    auto job = m_jobManager->activeQueue().object()->findJob(g_jobId);
    if (!job || job->state() != State::Completed)
    {
      return 1;
    }
    auto op = m_operationManager->create("smtk::job::CancelJob");
    op->parameters()->associate(job);
    auto result = op->operate();
    const bool rejected = result->findInt("outcome")->value() ==
      static_cast<int>(smtk::operation::Operation::Outcome::FAILED);
    if (!rejected || job->state() != State::Completed)
    {
      std::cerr << "ERROR: Canceling a completed job must fail without changing its state.\n";
      return 1;
    }
    return 0;
  }

  void changeActiveQueue(const smtk::string::Token qq)
  {
    smtk::job::Queue* queue{ nullptr };
    switch (qq.id())
    {
      case "smtk::qt::job::ShellQueue"_hash:
        queue = m_shellQueue.get();
        break;
      case "smtk::qt::job::ContainerQueue"_hash:
        queue = m_containerQueue.get();
        break;
    }
    m_jobManager->activeQueue().switchTo(queue);
  }

  bool ready() const { return m_ready; }

  int result() const { return m_result; }

  void cleanupCaseDirectories()
  {
    bool failed = false;
    for (const auto& caseDirectory : g_caseDirectories)
    {
      std::error_code ec;
      auto numRemoved = remove_all(caseDirectory, ec);
      if (ec || numRemoved == 0)
      {
        std::cerr << "ERROR: Could not remove case directory \"" << caseDirectory << "\".\n";
        failed = true;
        continue;
      }
    }
  }

protected:
  // The test result. Any value other than 0 is failure.
  // We initialize assuming failure so that if the test is interrupted
  // we report failure.
  int m_result{ 255 };
  bool m_ready{ false };
  bool m_phaseFinished{ false };
  bool m_cancelRequested{ false };
  int m_shellQueueUpdateKey{ 0 };
  int m_containerQueueUpdateKey{ 0 };
  std::shared_ptr<smtk::qt::job::ShellQueue> m_shellQueue;
  std::shared_ptr<smtk::qt::job::ContainerQueue> m_containerQueue;

  // Note the order of these members is important! Otherwise, registrars will not have
  // been called on m_appContext to insert objects needed by later registry objects:
  // clang-format off

  QCoreApplication* m_app{ nullptr };

  smtk::common::Managers::Ptr m_appContext;

  smtk::plugin::Registry<
    smtk::resource::Registrar,
    smtk::common::Managers>
      m_resourceRegistry;

  smtk::plugin::Registry<
    smtk::operation::Registrar,
    smtk::common::Managers>
      m_operationRegistry;

  smtk::resource::Manager::Ptr m_resourceManager;

  smtk::operation::Manager::Ptr m_operationManager;

  smtk::plugin::Registry<
    smtk::job::Registrar,
    smtk::common::Managers,
    smtk::resource::Manager,
    smtk::operation::Manager>
      m_jobRegistry;

  smtk::job::Manager::Ptr m_jobManager;

  smtk::plugin::Registry<
    smtk::extension::qtViewRegistrar,
    smtk::common::Managers,
    smtk::operation::Manager,
    smtk::resource::Manager,
    smtk::job::Manager>
      m_qtRegistry;

  QScopedPointer<QTimer> m_timer;
  // clang-format on
};

} // anonymous namespace

int jobQueue(int argc, char* argv[])
{
  smtk::io::Logger::instance().setFlushToStderr(true);
  QCoreApplication app(argc, argv);
  // Create a QObject we can "run" on the main thread:
  auto* jqt = new JobQueueTest(&app);
  if (!jqt->ready())
  {
    delete jqt;
    return 1;
  }

  // VM initialization and image pulls happen in the constructor above and may
  // exceed the per-phase timeout. Start a fresh watchdog only when a phase runs,
  // and stop it afterward so an earlier phase cannot time out a later one.
  QTimer watchdog;
  watchdog.setSingleShot(true);
  QObject::connect(&watchdog, &QTimer::timeout, &app, &QCoreApplication::quit);
  auto runPhase = [&]() {
    watchdog.start(15500);
    const int result = app.exec();
    const bool timedOut = !watchdog.isActive();
    watchdog.stop();
    if (timedOut)
    {
      std::cerr << "ERROR: Job queue test phase timed out.\n";
      return result + 1;
    }
    return result;
  };

  // -------------- ShellQueue tests
  std::cerr << "\n# ShellQueue tests\n\n";
  // I. Test that an operation creating a job causes the (auto-scheduled) job to run.
  // Schedule the test to run as soon as the event loop starts:
  std::cerr << "I. Auto-scheduled jobs are scheduled.\n\n";
  QTimer::singleShot(0, jqt, &JobQueueTest::test_basic);
  // Run until the application's quit() slot is invoked, then
  // grab the exit status from the test object.
  int status = runPhase();
  status += jqt->result();

  // II. Test that a job may be cancelled successfully.
  std::cerr << "\nII. Test canceling a running job succeeds.\n\n";
  jqt->reset();
  QTimer::singleShot(0, jqt, &JobQueueTest::test_cancel);
  // Run until the application's quit() slot is invoked, then
  // grab the exit status from the test object.
  status += runPhase();
  status += jqt->result();

  // III. Test that a job may that has been completed cannot be cancelled.
  std::cerr << "\nIII. Test canceling a completed job fails.\n\n";
  jqt->reset();
  QTimer::singleShot(0, jqt, &JobQueueTest::test_basic);
  // Run until the application's quit() slot is invoked, then
  // grab the exit status from the test object.
  status += runPhase();
  status += jqt->result();

  status += jqt->test_cancel_completed();

  // -------------- ContainerQueue tests
  std::cerr << "\n# ContainerQueue tests\n\n";
  jqt->changeActiveQueue("smtk::qt::job::ContainerQueue"_token);

  // IV. Test that a basic job runs
  // I. Test that an operation creating a job causes the (auto-scheduled) job to run.
  // Schedule the test to run as soon as the event loop starts:
  std::cerr << "\nIV. Auto-scheduled jobs are scheduled.\n\n";
  jqt->reset();
  QTimer::singleShot(0, jqt, &JobQueueTest::test_basic);
  // Run until the application's quit() slot is invoked, then
  // grab the exit status from the test object.
  status += runPhase();
  status += jqt->result();

  // V. Test that a job may be cancelled successfully.
  std::cerr << "\nV. Test canceling a running job succeeds.\n\n";
  jqt->reset();
  QTimer::singleShot(0, jqt, &JobQueueTest::test_cancel);
  // Run until the application's quit() slot is invoked, then
  // grab the exit status from the test object.
  status += runPhase();
  status += jqt->result();

  // Clean up and exit:
  jqt->cleanupCaseDirectories();
  delete jqt;
  // Normalize accumulated failures so an exit status of 256 cannot appear successful.
  return status == 0 ? 0 : 1;
}
