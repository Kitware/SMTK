//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/job/Definition.h"
#include "smtk/job/Job.h"
#include "smtk/job/Queue.h"
#include "smtk/job/operators/JobUpdated.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/common/testing/cxx/helpers.h"

int jobCanceledUpdates(int, char*[])
{
  using namespace smtk::job;
  auto queue = Queue::create();
  auto definition = Definition::create();
  definition->appendStage("Work", "Test progress updates after cancellation");
  auto job = Job::create();
  job->setQueue(queue.get());
  job->setJobType(definition.get());
  job->setStage(0);
  job->setState(State::Canceled);
  job->setStatus(Status::Terminated);

  // Reproduce a completion notification that was queued before cancellation,
  // but executes after CancelJob has assigned the terminal state and status.
  auto updater = JobUpdated::create();
  test(updater->parameters()->associate(job), "Associate the canceled job.");
  for (const auto& field : { "state", "status", "stage" })
  {
    updater->parameters()->findInt(field)->setIsEnabled(true);
  }
  updater->parameters()->findInt("state")->setValue(static_cast<int>(State::Completed));
  updater->parameters()->findInt("status")->setValue(static_cast<int>(Status::Succeeded));
  updater->parameters()->findInt("stage")->setValue(1);
  auto result = updater->operate();
  test(job->state() == State::Canceled, "Late completion must not overwrite cancellation.");
  test(job->status() == Status::Terminated, "Late completion must preserve terminated status.");
  test(job->stage() == 0, "Late progress must not advance a canceled job.");
  test(result->findComponent("modified")->numberOfValues() == 0, "Stale updates modify no jobs.");

  // Scheduling a new run resets the state; updates must then be accepted again.
  job->setState(State::Running);
  job->setStatus(Status::Pending);
  result = updater->operate();
  test(job->state() == State::Completed, "A new run can complete normally.");
  test(job->status() == Status::Succeeded, "A new run can succeed normally.");
  test(job->stage() == 1, "A new run can advance its stage.");
  return 0;
}
