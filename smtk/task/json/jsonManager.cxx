//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/json/jsonManager.h"
#include "smtk/task/Adaptor.h"
#include "smtk/task/Manager.h"
#include "smtk/task/Task.h"
#include "smtk/task/json/Helper.h"
#include "smtk/task/json/jsonAdaptor.h"
#include "smtk/task/json/jsonTask.h"

#include "smtk/io/Logger.h"

#include "nlohmann/json.hpp"

#include <exception>
#include <string>

namespace smtk
{
namespace task
{

void from_json(const nlohmann::json& jj, Manager& taskManager)
{
  try
  {
    auto& helper = json::Helper::instance();
    if (&taskManager != &helper.taskManager())
    {
      throw std::logic_error("Helper must be configured to deserialize this task manager.");
    }
    if (smtk::task::json::Helper::nestingDepth() == 1)
    { // Do not clear the parent task when deserializing nested tasks.
      helper.clear();
    }
    // helper.setManagers(managers);
    std::map<json::Helper::SwizzleId, Task::Ptr> taskMap;
    std::map<json::Helper::SwizzleId, Adaptor::Ptr> adaptorMap;
    for (const auto& jsonTask : jj.at("tasks"))
    {
      auto taskId = jsonTask.at("id").get<json::Helper::SwizzleId>();
      Task::Ptr task = jsonTask;
      taskMap[taskId] = task;
      helper.tasks().setSwizzleId(task.get(), taskId);
    }
    // Do a second pass to deserialize dependencies and UI config.
    for (const auto& jsonTask : jj.at("tasks"))
    {
      auto taskId = jsonTask.at("id").get<json::Helper::SwizzleId>();
      auto task = taskMap[taskId];
      if (jsonTask.contains("dependencies"))
      {
        auto taskDeps = helper.unswizzleDependencies(jsonTask.at("dependencies"));
        // Make sure this is not its own dependencies
        auto finder = taskDeps.find(task);
        if (finder != taskDeps.end())
        {
          smtkWarningMacro(
            smtk::io::Logger::instance(), task->title() << " trying to set deps to itself");
          taskDeps.erase(task);
        }

        task->addDependencies(taskDeps);
      }

      // Get UI object
      if (jsonTask.contains("ui"))
      {
        taskManager.uiState().setData(task, jsonTask["ui"]);
      }
    }

    // Now configure dependent tasks with adaptors if specified.
    // Note that tasks have already been deserialized, so the
    // helper's map from task-id to task-pointer is complete.
    if (jj.contains("adaptors"))
    {
      for (const auto& jsonAdaptor : jj.at("adaptors"))
      {
        // Skip things that are not adaptors.
        if (jsonAdaptor.is_object() && jsonAdaptor.contains("id"))
        {
          try
          {
            auto adaptorId = jsonAdaptor.at("id").get<json::Helper::SwizzleId>();
            auto taskFromId = jsonAdaptor.at("from").get<json::Helper::SwizzleId>();
            auto taskToId = jsonAdaptor.at("to").get<json::Helper::SwizzleId>();
            helper.setAdaptorTaskIds(taskFromId, taskToId);
            Adaptor::Ptr adaptor = jsonAdaptor;
            adaptorMap[adaptorId] = adaptor;
            helper.adaptors().swizzleId(adaptor.get());
          }
          catch (std::exception&)
          {
            smtkErrorMacro(
              smtk::io::Logger::instance(),
              "Skipping task because 'id', 'from', and/or 'to' fields are missing.");
          }
        }
      }
    }
    if (jj.contains("styles"))
    {
      taskManager.setStyles(jj.at("styles"));
    }

    // helper.clear();
  }
  catch (std::exception& e)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Could not deserialize: " << e.what() << ".");
  }
}

void to_json(nlohmann::json& jj, const Manager& manager)
{
  // Serialize tasks
  nlohmann::json::array_t taskList;
  manager.taskInstances().visit([&taskList](const smtk::task::Task::Ptr& task) {
    auto& helper = json::Helper::instance();
    if (
      !task->parent() ||
      (smtk::task::json::Helper::nestingDepth() > 1 &&
       helper.tasks().unswizzle(1) == task->parent()))
    {
      // Only serialize top-level tasks. (Tasks with children are responsible
      // for serializing their children).
      nlohmann::json jsonTask = task;
      if (!jsonTask.is_null())
      {
        helper.updateUIState(task, jsonTask);
        taskList.push_back(jsonTask);
      }
    }
    return smtk::common::Visit::Continue;
  });
  jj["tasks"] = taskList;

  // Serialize adaptors
  nlohmann::json::array_t adaptorList;
  manager.adaptorInstances().visit([&adaptorList](const smtk::task::Adaptor::Ptr& adaptor) {
    if (
      adaptor && adaptor->from() && !adaptor->from()->parent() && adaptor->to() &&
      !adaptor->to()->parent())
    {
      // Only serialize top-level adaptors. (Tasks with child adaptors are
      // responsible for serializing them.)
      nlohmann::json jsonAdaptor = adaptor;
      adaptorList.push_back(jsonAdaptor);
    }
    return smtk::common::Visit::Continue;
  });
  jj["adaptors"] = adaptorList;
  jj["styles"] = manager.getStyles();
}

} // namespace task
} // namespace smtk
