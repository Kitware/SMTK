//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/job/operators/CancelJob.h"

#include "smtk/job/operators/CancelJob_xml.h"

#include "smtk/job/Job.h"
#include "smtk/job/Queue.h"
#include "smtk/job/operators/ScheduleJob.h"

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

CancelJob::Result CancelJob::operateInternal()
{
  auto params = this->parameters();
  auto job = params->associations()->valueAs<smtk::job::Job>();
  if (!job || !job->queue())
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }
  // Only scheduled/running jobs may be canceled.
  auto jobState = job->state();
  if (jobState != smtk::job::State::Scheduled && jobState != smtk::job::State::Running)
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }
  // Cancel the job. This does not remove it from the queue.
  bool modified = job->queue()->cancel(job);
  modified |= job->setState(smtk::job::State::Canceled);
  auto result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  if (modified)
  {
    result->findComponent("modified")->appendValue(job);
  }
  return result;
}

void CancelJob::generateSummary(Operation::Result& /*unused*/) {}

const char* CancelJob::xmlDescription() const
{
  return CancelJob_xml;
}
} // namespace job
} // namespace smtk
