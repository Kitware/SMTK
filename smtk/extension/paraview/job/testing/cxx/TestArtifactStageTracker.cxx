//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/extension/paraview/job/ArtifactStageTracker.h"

/**\brief Verify which stage ranges require artifact refresh after job updates.
 *
 * A synthetic three-stage job exercises initial scheduling, stage completion,
 * repeated progress and success notifications, skipped intermediate updates,
 * rescheduling with the same UUID, and success without final stage progress.
 * A second job verifies that completion counts are tracked independently.
 * Failure and cancellation cases verify that only previously successful stages
 * are included and that repeated terminal notifications yield empty ranges.
 * Erasing tracking between terminal cases also checks that old counts are discarded.
 *
 * This is a unit test of ArtifactStageTracker's returned half-open ranges. It
 * does not launch jobs, dispatch operation observers, load artifact files, or
 * verify ParaView reader reloads, representation settings, or rendering.
 */
int TestArtifactStageTracker(int, char*[])
{
  using namespace smtk::job;
  smtk::extension::paraview::job::detail::ArtifactStageTracker tracker;
  auto definition = Definition::create();
  definition->appendStage("mesh", "Generate mesh");
  definition->appendStage("solve", "Run solver");
  definition->appendStage("report", "Generate report");
  auto job = Job::create();
  job->setJobType(definition);

  auto expect = [&](int first, int end, const char* message) {
    test(tracker.update(*job) == std::make_pair(first, end), message);
  };
  job->setState(Scheduled);
  job->setStatus(Pending);
  job->setStage(-1);
  expect(0, 0, "Scheduling must not reload artifacts.");
  job->setState(Running);
  job->setStage(0);
  expect(0, 0, "Starting the first stage must not reload artifacts.");
  job->setStage(1);
  expect(0, 1, "Completing the first stage must reload only its artifacts.");
  expect(1, 1, "Repeated progress must not reload earlier stages.");
  job->setStage(3);
  expect(1, 3, "Skipped notifications must include all newly completed stages.");
  job->setState(Completed);
  job->setStatus(Succeeded);
  expect(3, 3, "Success must not reload stages already handled by progress updates.");
  expect(3, 3, "Repeated success must not reload artifacts.");

  job->setState(Scheduled);
  job->setStatus(Pending);
  job->setStage(-1);
  expect(0, 0, "Rescheduling the same UUID must reset tracking.");
  job->setState(Running);
  job->setStage(2);
  expect(0, 2, "A rerun must refresh stages even if initial notifications were skipped.");
  job->setState(Completed);
  job->setStatus(Succeeded);
  expect(2, 3, "Success without final progress must refresh remaining stages.");

  auto other = Job::create();
  other->setJobType(definition);
  other->setState(Running);
  other->setStatus(Pending);
  other->setStage(1);
  test(tracker.update(*other) == std::make_pair(0, 1), "Jobs must have independent tracking.");
  expect(3, 3, "Updating another job must not reset this job.");

  tracker.erase(job->id());
  job->setStatus(Failed);
  job->setStage(2);
  expect(0, 1, "A failed stage must not be treated as successfully completed.");
  expect(1, 1, "Repeated failure must not reload completed stages.");
  tracker.erase(job->id());
  job->setStage(1);
  expect(0, 0, "Failure in the first stage must not reload artifacts.");

  tracker.erase(job->id());
  job->setState(Canceled);
  job->setStatus(Terminated);
  job->setStage(1);
  expect(0, 1, "Cancellation must refresh only stages completed before cancellation.");
  expect(1, 1, "Repeated cancellation must not reload artifacts.");
  return 0;
}
