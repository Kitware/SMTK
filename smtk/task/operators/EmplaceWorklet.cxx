//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/task/operators/EmplaceWorklet.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/ResourceItemDefinition.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"

#include "smtk/io/Logger.h"

#include "smtk/operation/Hints.h"
#include "smtk/operation/Manager.h"

#include "smtk/resource/Manager.h"

#include "smtk/project/Manager.h"
#include "smtk/project/json/jsonProject.h"

#include "smtk/task/json/Helper.h"
#include "smtk/task/json/jsonManager.h"
#include "smtk/task/json/jsonTask.h"

#include "smtk/task/operators/EmplaceWorklet_xml.h"

#include <string>

namespace smtk
{
namespace task
{

EmplaceWorklet::Result EmplaceWorklet::operateInternal()
{
  auto worklet = this->parameters()->associations()->valueAs<smtk::task::Worklet>();
  auto project = worklet ? dynamic_pointer_cast<smtk::project::Project>(worklet->resource())
                         : smtk::project::Project::Ptr();
  if (!project)
  {
    smtkErrorMacro(log(), "Associated worklet was null or had no parent project.");
    return this->createResult(Outcome::FAILED);
  }
  auto workletData = worklet->configuration();

  // Transcribe task data into the project's task manager.
  auto& taskHelper =
    smtk::task::json::Helper::pushInstance(project->taskManager(), this->managers());

  // Deserialize the project and see if it has an active task.
  smtk::task::from_json(workletData, project->taskManager());
  smtk::task::Task* taskToActivate = taskHelper.activeSerializedTask();

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  {
    auto created = result->findComponent("created");

    // Indicate that new tasks have been created:
    std::vector<smtk::task::Task*> tasks;
    std::vector<smtk::resource::Component::Ptr> sharedTasks;
    taskHelper.currentTasks(tasks);
    sharedTasks.reserve(tasks.size());
    for (const auto& task : tasks)
    {
      auto sharedTask = task->shared_from_this();
      sharedTasks.push_back(sharedTask);
    }
    created->appendValues(sharedTasks.begin(), sharedTasks.end());

    // Indicate that new adaptors have been created:
    std::vector<smtk::task::Adaptor*> adaptors;
    std::vector<smtk::resource::Component::Ptr> sharedAdaptors;
    taskHelper.currentAdaptors(adaptors);
    sharedAdaptors.reserve(adaptors.size());
    for (const auto& adaptor : adaptors)
    {
      auto sharedAdaptor = adaptor->shared_from_this();
      sharedAdaptors.push_back(sharedAdaptor);
    }
    created->appendValues(sharedAdaptors.begin(), sharedAdaptors.end());

    // Hint to the application to select and center on deserialized tasks:
    smtk::operation::addSelectionHint(result, sharedTasks);
    smtk::operation::addBrowserScrollHint(result, sharedTasks);

    // Hint to the application to make any deserialized task active upon completion:
    if (taskToActivate)
    {
      smtk::operation::addActivateTaskHint(result, project, taskToActivate);
    }
  }

  smtk::task::json::Helper::popInstance();

  return result;
}

const char* EmplaceWorklet::xmlDescription() const
{
  return EmplaceWorklet_xml;
}

} // namespace task
} // namespace smtk
