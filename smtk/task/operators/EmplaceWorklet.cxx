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

#include "smtk/view/Manager.h"
#include "smtk/view/UIElementState.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/ResourceItemDefinition.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/StringItemDefinition.h"

#include "smtk/operation/Hints.h"
#include "smtk/operation/Manager.h"

#include "smtk/resource/Manager.h"

#include "smtk/project/Manager.h"
#include "smtk/project/json/jsonProject.h"

#include "smtk/task/json/Helper.h"
#include "smtk/task/json/jsonManager.h"
#include "smtk/task/json/jsonTask.h"

#include "smtk/io/Logger.h"

#include "smtk/task/operators/EmplaceWorklet_xml.h"

#include <string>

using namespace smtk::string::literals;

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
  // Force transcription to rewrite UUIDs to newly-generated random IDs.
  // This is done so that users can drag and drop the same worklet into a project
  // multiple times without UUID collisions, while maintaining graphical layout,
  // port connections, and other inter-object references.
  taskHelper.setMapUUIDs(true);

  // Deserialize the worklet and see if it has an active task.
  smtk::task::from_json(workletData, project->taskManager());
  smtk::task::Task* taskToActivate = taskHelper.activeSerializedTask();

  // Handle any UIElementState, processing node (x,y) positions specially.
  auto it = workletData.find("ui_state");
  if (it != workletData.end())
  {
    auto viewMngr = this->managers()->get<smtk::view::Manager::Ptr>();
    if (viewMngr)
    {
      // for each UI element type specified, look to see if one is registered in the
      // View Manager and if so, pass it the configuration specified.
      auto& elementStateMap = viewMngr->elementStateMap();
      for (const auto& element : it->items())
      {
        auto it = elementStateMap.find(element.key());
        if (it != elementStateMap.end())
        {
          // Note: For backwards compatibility, we also handle pqSMTKTaskPanel data.
          if (element.key() == "pqSMTKDiagramPanel" || element.key() == "pqSMTKTaskPanel")
          {
            // Translate UI node layout now that tasks have UUIDs.
            // Copy the original:
            nlohmann::json taskPanelState = element.value();
            auto lit = taskPanelState.find("layout");
            auto locationItem = this->parameters()->findDouble("location");
            std::array<double, 2> dropPoint{ { locationItem->value(0), locationItem->value(1) } };
            std::array<double, 2> xy{ { 0, 0 } };
            int nxy = 0;
            if (lit != taskPanelState.end())
            {
              // Pass 1. Replace integer task IDs with newly-created UUIDs and
              //         computer average nodal position.
              for (auto& entry : *lit)
              {
                auto* obj = taskHelper.objectFromJSONSpec(entry[0], "task"_token);
                if (obj)
                {
                  entry[0] = obj->id().toString();
                }
                if (auto* task = dynamic_cast<smtk::task::Task*>(obj))
                {
                  // Only include top-level task nodes in average coordinate.
                  if (!task->parent())
                  {
                    for (int ii = 0; ii < 2; ++ii)
                    {
                      xy[ii] += entry[1][ii].get<double>();
                    }
                    ++nxy;
                  }
                }
              }
              // Compute adjustment to task node locations based on drop point:
              // std::cout << "Average worklet pt  " << (xy[0]/nxy) << " " << (xy[1]/nxy) << "\n";
              for (int ii = 0; ii < 2; ++ii)
              {
                xy[ii] = dropPoint[ii] - xy[ii] / static_cast<double>(nxy);
              }
              // std::cout << "Adjust worklets by " << (xy[0]/nxy) << " " << (xy[1]/nxy) << "\n";
              // Pass 2. Adjust nodal position so average lies at drop point.
              for (auto& entry : *lit)
              {
                for (int ii = 0; ii < 2; ++ii)
                {
                  entry[1][ii] = entry[1][ii].get<double>() + xy[ii];
                }
                // std::cout << "            Task @ " << entry[1][0] << " " << entry[1][1] << "\n";
              }
            }
            if (!it->second->configure(taskPanelState))
            {
              smtkErrorMacro(
                log(), "ElementState " << element.key() << " failed to be configured.");
            }
          }
          else
          {
            if (!it->second->configure(element.value()))
            {
              smtkErrorMacro(
                log(), "ElementState " << element.key() << " failed to be configured.");
            }
          }
        }
      }
    }
  }

  // Now that the worklet has been emplaced, reset the task-helper so
  // objects do not have their UUIDs mapped.
  taskHelper.setMapUUIDs(false);

  // See if the created tasks are to be children
  smtk::task::TaskPtr parentTask;
  auto parentTaskItem = this->parameters()->findComponent("parentTask");
  if (parentTaskItem->isEnabled())
  {
    parentTask = std::dynamic_pointer_cast<smtk::task::Task>(parentTaskItem->value());
  }

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
    if (parentTask)
    {
      if (!parentTask->acceptsChildCategories(worklet->categories()))
      {
        smtkWarningMacro(
          smtk::io::Logger::instance(),
          "Worklet: " << worklet->name() << "'s categories did not pass Task: "
                      << parentTask->name() << "'s category constraints.\n");
      }
      for (const auto& task : sharedTasks)
      {
        parentTask->addChild(std::dynamic_pointer_cast<smtk::task::Task>(task));
      }
      auto modified = result->findComponent("modified");
      modified->appendValue(parentTask);
    }
    else if (!project->taskManager().toplevelExpression().passes(worklet->categories()))
    {
      smtkWarningMacro(
        smtk::io::Logger::instance(),
        "Worklet: "
          << worklet->name()
          << "'s categories did not pass the workflow's top-level category expression.\n");
    }
    created->appendValues(sharedTasks.begin(), sharedTasks.end());

    // Indicate that new ports have been created:
    std::vector<smtk::task::Port*> ports;
    std::vector<smtk::resource::Component::Ptr> sharedPorts;
    taskHelper.currentPorts(ports);
    sharedPorts.reserve(ports.size());
    for (const auto& port : ports)
    {
      auto sharedPort = port->shared_from_this();
      sharedPorts.push_back(sharedPort);
    }
    created->appendValues(sharedPorts.begin(), sharedPorts.end());

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
