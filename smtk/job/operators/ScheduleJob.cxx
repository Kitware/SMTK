//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/job/operators/ScheduleJob.h"

#include "smtk/job/operators/ScheduleJob_xml.h"

#include "smtk/job/Job.h"
#include "smtk/job/Queue.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/io/Logger.h"

namespace smtk
{
namespace job
{

ScheduleJob::Result ScheduleJob::operateInternal()
{
  auto params = this->parameters();
  auto job = params->associations()->valueAs<smtk::job::Job>();
  if (!job || !job->queue())
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }
  // Schedule the job.
  bool modified = false;
  if (job->stage() >= 0)
  {
    // Prepare to re-run a job that has already run.
    modified |= job->setStage(-1);
    modified |= job->setState(smtk::job::Unscheduled);
    modified |= job->setStatus(smtk::job::Pending);
    std::filesystem::remove(job->caseDirectory() / "logs" / "progress");
  }
  bool scheduled = job->queue()->schedule(job);

  auto result = this->createResult(
    scheduled ? smtk::operation::Operation::Outcome::SUCCEEDED
              : smtk::operation::Operation::Outcome::FAILED);
  if (modified || scheduled)
  {
    result->findComponent("modified")->appendValue(job);
  }
  return result;
}

void ScheduleJob::generateSummary(Operation::Result& /*unused*/) {}

const char* ScheduleJob::xmlDescription() const
{
  return ScheduleJob_xml;
}
} // namespace job
} // namespace smtk
