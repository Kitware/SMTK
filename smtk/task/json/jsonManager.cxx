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
#include "smtk/task/json/jsonWorklet.h"

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
    std::map<smtk::common::UUID, Task::Ptr> taskMap;
    std::map<json::Helper::SwizzleId, Adaptor::Ptr> adaptorMap;
    for (const auto& jsonTask : jj.at("tasks"))
    {
      json::Helper::SwizzleId taskSwizzle = 0;
      auto taskId = smtk::common::UUID::null();
      auto jTaskId = jsonTask.at("id");
      if (jTaskId.is_number_integer())
      {
        taskSwizzle = jTaskId.get<json::Helper::SwizzleId>();
      }
      else if (jTaskId.is_string())
      {
        taskId = jsonTask.at("id").get<smtk::common::UUID>();
      }
      Task::Ptr task = jsonTask;
      if (taskSwizzle)
      {
        helper.tasks().setSwizzleId(task.get(), taskSwizzle);
        taskId = task->id(); // task->setId(taskId);
      }
      else
      {
        taskSwizzle = helper.tasks().swizzleId(task.get());
      }
      taskMap[taskId] = task;
    }
    // Do a second pass to deserialize dependencies and UI config.
    for (const auto& jsonTask : jj.at("tasks"))
    {
      auto jTaskId = jsonTask.at("id");
      Task::Ptr task;
      if (jTaskId.is_number_integer())
      {
        auto taskSwizzle = jTaskId.get<json::Helper::SwizzleId>();
        auto* tp = helper.tasks().unswizzle(taskSwizzle);
        task = tp ? std::static_pointer_cast<smtk::task::Task>(tp->shared_from_this()) : nullptr;
      }
      else
      {
        auto taskId = jsonTask.at("id").get<smtk::common::UUID>();
        auto it = taskMap.find(taskId);
        if (it != taskMap.end())
        {
          task = it->second;
        }
      }
      if (!task)
      {
        smtkErrorMacro(
          smtk::io::Logger::instance(), "Could not find task for id: " << jTaskId << ".");
        continue;
      }
      if (jsonTask.contains("dependencies"))
      {
        smtk::task::Task::PassedDependencies taskDeps;
        for (const auto& dep : jsonTask.at("dependencies"))
        {
          if (dep.is_number_integer())
          {
            // Set by call to helper.tasks().setSwizzleId(task.get(), taskId) above
            auto* depTask = helper.tasks().unswizzle(dep.get<json::Helper::SwizzleId>());
            if (depTask)
            {
              taskDeps.insert(
                std::static_pointer_cast<smtk::task::Task>(depTask->shared_from_this()));
            }
            else
            {
              smtkErrorMacro(
                smtk::io::Logger::instance(),
                task->name() << " has unknown dependency " << dep.get<json::Helper::SwizzleId>()
                             << "; skipping.");
              continue;
            }
          }
          else
          {
            auto it = taskMap.find(dep.get<smtk::common::UUID>());
            if (it == taskMap.end())
            {
              smtkErrorMacro(
                smtk::io::Logger::instance(),
                task->name() << " has unknown dependency " << dep << "; skipping.");
              continue;
            }
            if (it->second == task)
            {
              smtkWarningMacro(
                smtk::io::Logger::instance(), task->name() << " trying to set deps to itself");
              continue;
            }
            taskDeps.insert(it->second);
          }
        }
        task->addDependencies(taskDeps);
      }
      // TODO: UI state.
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
            auto jAdaptorId = jsonAdaptor.at("id");
            json::Helper::SwizzleId adaptorId;
            if (jAdaptorId.is_string())
            {
              adaptorId = -128;
            }
            else if (jAdaptorId.is_number_integer())
            {
              adaptorId = jsonAdaptor.at("id").get<json::Helper::SwizzleId>();
            }
            // Tasks are now components. They may have temporary IDs that need to
            // be de-swizzled. Or they may just have UUIDs.
            auto jTaskFrom = jsonAdaptor.at("from");
            auto jTaskTo = jsonAdaptor.at("to");
            if (jTaskFrom.is_string() && jTaskTo.is_string())
            {
              // Both tasks specified by UUID.
              auto taskFromId = jTaskFrom.get<smtk::common::UUID>();
              auto taskToId = jTaskTo.get<smtk::common::UUID>();
              helper.setAdaptorTaskIds(taskFromId, taskToId);
            }
            else if (jTaskFrom.is_number_integer() && jTaskTo.is_number_integer())
            {
              // Both tasks specified by integer index (not a UUID).
              auto taskFromId = jTaskFrom.get<json::Helper::SwizzleId>();
              auto taskToId = jTaskTo.get<json::Helper::SwizzleId>();
              helper.setAdaptorTaskIds(taskFromId, taskToId);
            }
            else
            {
              smtkErrorMacro(
                smtk::io::Logger::instance(),
                "Skipping adaptor because 'from' and 'to' are not of the same, proper type.");
              continue;
            }
            Adaptor::Ptr adaptor = jsonAdaptor;
            adaptorId = helper.adaptors().swizzleId(adaptor.get());
            adaptorMap[adaptorId] = adaptor;
          }
          catch (std::exception& e)
          {
            std::cout << e.what() << "ERROR\n";
            smtkErrorMacro(
              smtk::io::Logger::instance(),
              "Skipping adaptor because 'id', 'from', and/or 'to' fields are missing.");
          }
        }
      }
    }
    if (jj.contains("styles"))
    {
      taskManager.setStyles(jj.at("styles"));
    }

    // Read in the Worklet Gallery if one exists
    auto it = jj.find("gallery");
    if (it != jj.end())
    {
      std::vector<Worklet::Ptr> worklets;
      worklets = it->get<std::vector<Worklet::Ptr>>();
      for (const auto& worklet : worklets)
      {
        if (!taskManager.gallery().add(worklet))
        {
          smtkErrorMacro(
            smtk::io::Logger::instance(),
            "Could not add Worklet: " << worklet->name() << " to TaskManager's Gallery");
        }
      }
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
        // helper.updateUIState(task, jsonTask);
        taskList.push_back(jsonTask);
      }
    }
    return smtk::common::Visit::Continue;
  });
  jj["tasks"] = taskList;

  // Serialize adaptors
  nlohmann::json::array_t adaptorList;
  adaptorList.reserve(manager.adaptorInstances().size());
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

  // Serialize the Worklet gallery if one exists
  const std::unordered_map<smtk::string::Token, Worklet::Ptr>& gallery =
    manager.gallery().worklets();
  if (!gallery.empty())
  {
    std::vector<Worklet::Ptr> worklets;
    worklets.reserve(gallery.size());
    for (const auto& workletInfo : gallery)
    {
      worklets.push_back(workletInfo.second);
    }
    jj["gallery"] = worklets;
  }
}

} // namespace task
} // namespace smtk
