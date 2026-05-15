//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/job/Resource.h"

#include "smtk/extension/qt/qtViewRegistrar.h"

#include "smtk/attribute/ComponentItem.h"
#include "smtk/common/Managers.h"
#include "smtk/io/Logger.h"
#include "smtk/job/Definition.h"
#include "smtk/job/Job.h"
#include "smtk/job/Manager.h"
#include "smtk/job/Registrar.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"
#include "smtk/operation/Registrar.h"
#include "smtk/plugin/Registry.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Registrar.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <thread>

namespace
{

using namespace smtk::job;

Queue* defaultQueue = nullptr;

std::mutex opDoneLock;
std::condition_variable opDoneCondition;
bool opDone = false;

smtk::common::UUID jobId;

// The "job" to perform.
// TODO: This should test that an input file exists and generate
//       an output file from it (in addition to "logs/job.log").
// TODO: This should test concurrency with mpiexec (or srun, etc.).
std::string job_script_text = R"(#!/bin/bash
echo Foo
echo "0" > logs/progress
echo "not yet" > logs/job.log
sleep 2
echo "wait for it" >> logs/job.log
echo "1" > logs/progress
echo "done" >> logs/job.log
sleep 1
echo "-2" > logs/progress
)";

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
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1500ms);

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
    this->writeFile(caseDir, jobDef->script(), job_script_text);

    job->setId(smtk::common::UUID::random());
    job->setSize(1); // Don't run in parallel
    job->setQueue(defaultQueue);
    job->setCaseDirectory(caseDir);

    // Set a global variable with the created job ID so the test can find it.
    jobId = job->id();

    auto result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
    auto jobsItem = result->findComponent("jobsToSubmit");
    jobsItem->appendValue(job);
    return result;
  }

  Specification createSpecification() override
  {
    auto spec = this->createBaseSpecification();
    spec->createDefinition("JobCreatorOp", "operation");
    spec->createDefinition("result(JobCreatorOp)", "result");
    return spec;
  }
};

} // anonymous namespace

int jobResourceInstance(int /*unused*/, char* /*unused*/[])
{
  // Construct smtk managers and invoke various Registrar classes.
  auto appContext = smtk::common::Managers::create();
  auto commonRegistry = smtk::plugin::addToManagers<smtk::resource::Registrar>(appContext);
  auto resourceRegistry = smtk::plugin::addToManagers<smtk::resource::Registrar>(appContext);
  auto operationRegistry = smtk::plugin::addToManagers<smtk::operation::Registrar>(appContext);

  // Fetch resource and operation managers from the application context.
  auto resourceManager = appContext->get<smtk::resource::Manager::Ptr>();
  auto operationManager = appContext->get<smtk::operation::Manager::Ptr>();
  auto jobRegistry = smtk::plugin::addToManagers<smtk::job::Registrar>(
    appContext, resourceManager, operationManager);
  auto jobManager = appContext->get<smtk::job::Manager::Ptr>();
  auto viewRegistry = smtk::plugin::addToManagers<smtk::extension::qtViewRegistrar>(
    appContext, operationManager, resourceManager, jobManager);

  // Create a new type of job and register it.
  auto jobType = smtk::job::Definition::create();
  std::filesystem::path scriptPath = "run_job.sh";
  std::filesystem::path logPath = "logs/job.log";
  jobType->setName("Test");
  // We have two stages:
  int stageIdx = jobType->appendStage("Pretending", "Pretend to do work", logPath);
  stageIdx = jobType->appendStage("Hallucinating", "Hallucinate results", logPath);
  jobManager->jobTypes().manage(jobType);

  // Create our test operation whose result includes a job to queue:
  operationManager->registerOperation<JobCreatorOp>();

  // Fetch the singleton "job" resource:
  auto resource = smtk::job::Resource::instance();
  if (!resource)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "No job resource.");
    return 1;
  }

  defaultQueue = jobManager->defaultQueue();

  // Because this test must work for developer builds as well as on CI machines and
  // because the job resource is a singleton that may host other running jobs, we
  // only look for the job to make sure it has been added.
  // The job has delays that mostly guarantee it will still be running once the
  // signal from the operation handler pings the main thread.
  auto jobOp = operationManager->create<JobCreatorOp>();
  jobOp->addHandler(
    [&](smtk::operation::Operation&, const JobCreatorOp::Result& result) {
      std::cerr << "Operation completed " << static_cast<int>(smtk::operation::outcome(result))
                << "\n";
      {
        std::unique_lock<std::mutex> opLock(opDoneLock);
        opDone = true;
        opDoneCondition.notify_one();
      }
    },
    0);
  operationManager->launchers()(jobOp);
  {
    // Wait for the operation to complete (which queues the job).
    std::unique_lock<std::mutex> guard(opDoneLock);
    opDoneCondition.wait(guard, [] { return opDone; });
  }
  {
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(500ms);
  }
  auto job = std::static_pointer_cast<smtk::job::Job>(resource->find(jobId));
  {
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(5000ms);
  }
  std::cerr << "Expected to find new job, got " << job << " for " << jobId.toString() << "\n";
  if (!job)
  {
    std::cerr << "ERROR: Job not found.\n";
    return 1;
  }

  auto caseDirectory = job->caseDirectory();

  // Wait for the job script to finish running.
  // Make sure job events were observed.

  // Remove the temporary case directory.
  std::error_code ec;
  auto numRemoved = remove_all(caseDirectory, ec);
  if (ec || numRemoved == 0)
  {
    std::cerr << "ERROR: Could not remove case directory \"" << caseDirectory << "\".\n";
    return 1;
  }
  return 0;
}
