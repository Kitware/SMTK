//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/job/operators/AddJobToQueue.h"

#include "smtk/job/operators/AddJobToQueue_xml.h"

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

AddJobToQueue::Result AddJobToQueue::operateInternal()
{
  auto params = this->parameters();
  auto job = params->associations()->valueAs<smtk::job::Job>();
  if (!job || !job->queue())
  {
    std::cerr << "ERROR: FAILED TO ADD JOB\n";
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }
  // Add the job to the queue. This does not schedule it.
  bool modified = job->queue()->add(job);
  modified |= job->setState(smtk::job::State::Unscheduled);
  std::cerr << "ADDED JOB\n";
  if (job->autoSchedule())
  {
    auto operationManager = this->managers()->get<smtk::operation::Manager::Ptr>();
    if (operationManager)
    {
      std::cerr << "SCHEDULING JOB\n";
      if (auto op = operationManager->create<smtk::job::ScheduleJob>())
      {
        op->parameters()->associate(job);
        operationManager->launchers()(op);
      }
    }
    else
    {
      std::cerr << "SCHEDULING JOB xxxx\n";
      modified |= job->queue()->schedule(job);
    }
  }

  auto result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  if (modified)
  {
    result->findComponent("modified")->appendValue(job);
  }
  return result;
}

void AddJobToQueue::generateSummary(Operation::Result& /*unused*/) {}

const char* AddJobToQueue::xmlDescription() const
{
  return AddJobToQueue_xml;
}
} // namespace job
} // namespace smtk
