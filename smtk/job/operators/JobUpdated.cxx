//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/job/operators/JobUpdated.h"

#include "smtk/job/operators/JobUpdated_xml.h"

#include "smtk/job/DatabaseQueue.h"
#include "smtk/job/Job.h"

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

JobUpdated::Result JobUpdated::operateInternal()
{
  auto params = this->parameters();
  auto job = params->associations()->valueAs<smtk::job::Job>();
  if (!job || !job->queue())
  {
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }
  auto result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  // Modify the job as directed, optionally adding it to the result's list of
  // modified components (if the job was in fact changed).
  auto stateItem = params->findInt("state");
  auto statusItem = params->findInt("status");
  auto stageItem = params->findInt("stage");
  bool didModify = false;
  auto prevState = job->state();
  auto prevStatus = job->status();
  if (stageItem->isEnabled())
  {
    if (job->stage() > stageItem->value())
    {
      // This notification is stale: the stage counter should never decrement.
      return this->createResult(smtk::operation::Operation::Outcome::FAILED);
    }
    if (job->setStage(stageItem->value()))
    {
      didModify = true;
    }
  }
  if (stateItem->isEnabled())
  {
    if (stateItem->value() >= job->state())
    {
      if (job->setState(static_cast<smtk::job::State>(stateItem->value())))
      {
        didModify = true;
      }
    }
  }
  if (statusItem->isEnabled())
  {
    if (statusItem->value() >= job->status())
    {
      if (job->setStatus(static_cast<smtk::job::Status>(statusItem->value())))
      {
        didModify = true;
      }
    }
  }
  if (didModify)
  {
    result->findComponent("modified")->appendValue(job);
    if (auto queue = dynamic_cast<DatabaseQueue*>(job->queue()))
    {
      queue->updateJobDatabaseInfo(job);
    }
  }

  return result;
}

void JobUpdated::generateSummary(Operation::Result& result)
{
  auto status = smtk::operation::outcome(result);
  if (status != Outcome::SUCCEEDED)
  {
    this->Superclass::generateSummary(result);
  }

  auto modItem = result->findComponent("modified");
  if (modItem && !modItem->empty())
  {
    if (auto job = modItem->valueAs<smtk::job::Job>())
    {
      smtkInfoMacro(
        this->log(),
        "Job " + job->name() + " updated: " + smtk::job::stateAsString(job->state()) + " " +
          smtk::job::statusAsString(job->status()) + " stage " + std::to_string(job->stage()) +
          ".");
      return;
    }
  }
  smtkWarningMacro(this->log(), "No job scheduled.");
}

const char* JobUpdated::xmlDescription() const
{
  return JobUpdated_xml;
}
} // namespace job
} // namespace smtk
