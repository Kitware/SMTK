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
namespace json
{

bool jsonManager::serialize(
  const std::shared_ptr<smtk::common::Managers>& managers,
  nlohmann::json& json)
{
  auto taskManager = managers->get<smtk::task::Manager::Ptr>();
  if (!taskManager)
  {
    // Should we succeed silently instead of failing verbosely?
    smtkErrorMacro(
      smtk::io::Logger::instance(),
      "Could not find a task manager to serialize. "
      "Add a task manager to the managers instance.");
    return false;
  }

  // Serialize tasks
  nlohmann::json::array_t taskList;
  taskManager->taskInstances().visit([&taskList](const smtk::task::Task::Ptr& task) {
    nlohmann::json jsonTask = task;
    taskList.push_back(jsonTask);
    return smtk::common::Visit::Continue;
  });
  json["tasks"] = taskList;

  // Serialize adaptors
  nlohmann::json::array_t adaptorList;
  taskManager->adaptorInstances().visit([&adaptorList](const smtk::task::Adaptor::Ptr& adaptor) {
    nlohmann::json jsonAdaptor = adaptor;
    adaptorList.push_back(jsonAdaptor);
    return smtk::common::Visit::Continue;
  });
  json["adaptors"] = adaptorList;
  return true;
}

bool jsonManager::deserialize(
  const std::shared_ptr<smtk::common::Managers>& managers,
  const nlohmann::json& json)
{
  auto taskManager = managers->get<smtk::task::Manager::Ptr>();
  if (!taskManager)
  {
    // Should we succeed silently instead of failing verbosely?
    smtkErrorMacro(smtk::io::Logger::instance(), "Could not find a destination task manager.");
    return false;
  }

  try
  {
    auto& helper = Helper::instance();
    helper.clear();
    helper.setManagers(managers);
    std::map<std::size_t, Task::Ptr> taskMap;
    for (const auto& jsonTask : json.at("tasks"))
    {
      auto taskId = jsonTask.at("id").get<std::size_t>();
      Task::Ptr task = jsonTask;
      taskMap[taskId] = task;
      helper.tasks().swizzleId(task.get());
    }
    // Do a second pass to deserialize dependencies.
    for (const auto& jsonTask : json.at("tasks"))
    {
      if (jsonTask.contains("dependencies"))
      {
        auto taskId = jsonTask.at("id").get<std::size_t>();
        auto task = taskMap[taskId];
        auto taskDeps = helper.unswizzleDependencies(jsonTask.at("dependencies"));
        task->addDependencies(taskDeps);
      }
    }
    // Now configure dependent tasks with adaptors if specified.
    // Note that tasks have already been deserialized, so the
    // helper's map from task-id to task-pointer is complete.
    if (json.contains("adaptors"))
    {
      for (const auto& jsonAdaptor : json.at("adaptors"))
      {
        try
        {
          auto adaptorId = jsonAdaptor.at("id").get<std::size_t>();
          auto taskFromId = jsonAdaptor.at("from").get<std::size_t>();
          auto taskToId = jsonAdaptor.at("to").get<std::size_t>();
          helper.setAdaptorTaskIds(taskFromId, taskToId);
          Adaptor::Ptr adaptor = jsonAdaptor;
          helper.clearAdaptorTaskIds();
        }
        catch (std::exception&)
        {
          smtkErrorMacro(
            smtk::io::Logger::instance(),
            "Skipping task because 'id', 'from', and/or 'to' fields are missing.");
        }
      }
    }

    helper.clear();
  }
  catch (std::exception& e)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Could not deserialize: " << e.what() << ".");
    return false;
  }
  return true;
}

} // namespace json
} // namespace task
} // namespace smtk
