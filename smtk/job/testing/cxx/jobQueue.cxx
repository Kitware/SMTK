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
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"

#include "smtk/attribute/ComponentItem.h"
#include "smtk/io/Logger.h"

#include "smtk/plugin/Registry.h"

#include "smtk/resource/Manager.h"
#include "smtk/resource/Registrar.h"

#include "smtk/common/Managers.h"
#include "smtk/common/UUID.h"

#include "smtk/string/Token.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QObject>
#include <QThread>
#include <QTimer>

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
echo "0" > logs/progress
echo "not yet" > logs/job.log
# echo TestJob: Progress initialized
sleep 0.00625
echo "wait for it" >> logs/job.log
echo "1" > logs/progress
# echo TestJob: Stage 1 complete
echo "done" >> logs/job.log
sleep 0.00625
# echo TestJob: Stage 2 complete
echo "2" > logs/progress
)foo";

std::string job_script_cancel_text = R"foo(#!/bin/bash
# echo TestJob: starting
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd $SCRIPT_DIR
echo "0" > logs/progress
echo "not yet" > logs/job.log
# echo TestJob: Progress initialized
sleep 2
echo "wait for it" >> logs/job.log
echo "1" > logs/progress
# echo TestJob: Stage 1 complete
echo "done" >> logs/job.log
sleep 20
# echo TestJob: Stage 2 complete
echo "2" > logs/progress
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
    auto tempDir = std::filesystem::temp_directory_path();
    std::string pattern = (tempDir / "smtkXXXXXX").string();
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

    g_jobId = job->id();
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
    op->parameters()->associate(m_containerQueue);
    op->operate();
    // Pull an image to run.
    std::cout << "  Pulling image ubuntu:26.04\n";
    m_containerQueue->pullContainerImage("ubuntu:26.04");
    std::cout << "    done\n";

    m_jobManager->queues().manage(m_shellQueue);
    m_jobManager->queues().manage(m_containerQueue);
    m_jobManager->activeQueue().switchTo(m_shellQueue.get());

    // Add a job observer to update JobQueueTest with a result
    // as AddJobToQueue and JobUpdated are invoked in response to JobCreatorOp.
    m_shellQueueUpdateKey = m_shellQueue->observe(nullptr, this->jobStateObserver(), false);
    m_containerQueueUpdateKey = m_containerQueue->observe(nullptr, this->jobStateObserver(), false);
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

      // If we are done, stop the event loop:
      if (
        job.stage() == job.jobType()->stages().size() ||
        job.status() == smtk::job::Status::Terminated)
      {
        // Unobserve this job
        // This needs to happen before we erase the job directory
        // or the ShellQueue will report status changes as we exit.
        // It is still possible that a JobUpdate operation has been
        // launched but not run. Handle that separately below.
        if (job.queue()->matchesType("smtk::qt::job::ShellQueue"_token))
        {
          job.queue()->unobserve(const_cast<Job*>(&job), m_shellQueueUpdateKey);
        }
        else
        {
          job.queue()->unobserve(const_cast<Job*>(&job), m_containerQueueUpdateKey);
        }

        // Remember the temporary case directory so we can remove it on cleanup.
        auto caseDirectory = job.caseDirectory();
        g_caseDirectories.insert(caseDirectory);

        // Check that the final job state matches what we expect.
        if (
          (g_expectToCancel && job.state() == State::Canceled) ||
          (!g_expectToCancel && job.state() == State::Completed))
        {
          m_result = 0;
        }
        else
        {
          std::cerr << "ERROR: Unexpected end state " << smtk::job::stateAsString(job.state())
                    << "\n";
          m_result = 56;
        }

        // This is needed to call the slot from a non-Qt thread:
        QMetaObject::invokeMethod(m_timer.data(), "start", Qt::QueuedConnection);
      }
    };
  }
  void reset() { m_result = 255; }

public Q_SLOTS:

  void test_basic()
  {
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
  // Schedule a timeout to cancel the event loop if the test fails.
  // Comment this out when debugging.
  QTimer::singleShot(15500, &app, &QCoreApplication::quit);

  // Create a QObject we can "run" on the main thread:
  auto* jqt = new JobQueueTest(&app);

  // -------------- ShellQueue tests
  // I. Test that an operation creating a job causes the (auto-scheduled) job to run.
  // Schedule the test to run as soon as the event loop starts:
  QTimer::singleShot(0, jqt, &JobQueueTest::test_basic);
  // Run until the application's quit() slot is invoked, then
  // grab the exit status from the test object.
  int status = app.exec();
  status += jqt->result();

  // II. Test that a job may be cancelled successfully.
  jqt->reset();
  QTimer::singleShot(0, jqt, &JobQueueTest::test_cancel);
  QTimer::singleShot(200, jqt, &JobQueueTest::cancel_job);
  // Run until the application's quit() slot is invoked, then
  // grab the exit status from the test object.
  status += app.exec();
  status += jqt->result();

  // III. Test that a job may that has been completed cannot be cancelled.
  jqt->reset();
  QTimer::singleShot(0, jqt, &JobQueueTest::test_basic);
  QTimer::singleShot(500, jqt, &JobQueueTest::cancel_job);
  // Run until the application's quit() slot is invoked, then
  // grab the exit status from the test object.
  status += app.exec();
  status += jqt->result();

  // -------------- ContainerQueue tests
  jqt->changeActiveQueue("smtk::qt::job::ContainerQueue"_token);

  // IV. Test that a basic job runs
  // I. Test that an operation creating a job causes the (auto-scheduled) job to run.
  // Schedule the test to run as soon as the event loop starts:
  jqt->reset();
  QTimer::singleShot(0, jqt, &JobQueueTest::test_basic);
  // Run until the application's quit() slot is invoked, then
  // grab the exit status from the test object.
  status += app.exec();
  status += jqt->result();

  // V. Test that a job may be cancelled successfully.
  jqt->reset();
  QTimer::singleShot(0, jqt, &JobQueueTest::test_cancel);
  QTimer::singleShot(1000, jqt, &JobQueueTest::cancel_job);
  // Run until the application's quit() slot is invoked, then
  // grab the exit status from the test object.
  status += app.exec();
  status += jqt->result();

  // Clean up and exit:
  jqt->cleanupCaseDirectories();
  delete jqt;
  return status;
}
