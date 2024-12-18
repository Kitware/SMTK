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
#include "smtk/task/Port.h"
#include "smtk/task/Task.h"
#include "smtk/task/json/Helper.h"
#include "smtk/task/json/jsonAdaptor.h"
#include "smtk/task/json/jsonPort.h"
#include "smtk/task/json/jsonTask.h"
#include "smtk/task/json/jsonWorklet.h"

#include "smtk/resource/Manager.h"

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
    std::map<smtk::common::UUID, Port::Ptr> portMap;
    std::map<json::Helper::SwizzleId, Adaptor::Ptr> adaptorMap;
    // Deserialize ports first so tasks/agents can find them upon initial
    // configuration.
    if (jj.contains("ports"))
    {
      for (const auto& jsonPort : jj.at("ports"))
      {
        json::Helper::SwizzleId portSwizzle = 0;
        auto portId = smtk::common::UUID::null();
        auto jPortId = jsonPort.at("id");
        if (jPortId.is_number_integer())
        {
          portSwizzle = jPortId.get<json::Helper::SwizzleId>();
        }
        else if (jPortId.is_string())
        {
          portId = jsonPort.at("id").get<smtk::common::UUID>();
        }
        Port::Ptr port = jsonPort;
        if (portSwizzle)
        {
          helper.ports().setSwizzleId(port.get(), portSwizzle);
          portId = port->id(); // port->setId(portId);
        }
        else
        {
          portSwizzle = helper.ports().swizzleId(port.get());
        }
        portMap[portId] = port;
      }
    }
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
    // Do a second pass on ports to deserialize connections and UI config.
    // All objects in a port's connections should now exist (tasks have
    // been created and a project should load its child resources before
    // deserializing the task manager, so external connections are also OK).
    if (jj.contains("ports"))
    {
      for (const auto& jsonPort : jj.at("ports"))
      {
        auto jPortId = jsonPort.at("id");
        Port::Ptr port;
        if (jPortId.is_number_integer())
        {
          auto portSwizzle = jPortId.get<json::Helper::SwizzleId>();
          auto* tp = helper.ports().unswizzle(portSwizzle);
          port = tp ? std::static_pointer_cast<smtk::task::Port>(tp->shared_from_this()) : nullptr;
        }
        else
        {
          auto portId = jsonPort.at("id").get<smtk::common::UUID>();
          auto it = portMap.find(portId);
          if (it != portMap.end())
          {
            port = it->second;
          }
        }
        if (!port)
        {
          smtkErrorMacro(
            smtk::io::Logger::instance(), "Could not find port for id: " << jPortId << ".");
          continue;
        }
        auto* portRsrc = port->parentResource();
        if (jsonPort.contains("connections"))
        {
          auto resourceManager = helper.managers()->get<smtk::resource::Manager::Ptr>();
          const auto& jpconn(jsonPort.at("connections"));
          port->connections().reserve(jpconn.size());
          for (const auto& dep : jsonPort.at("connections"))
          {
            if (dep.is_number_integer())
            {
              // Set by call to helper.ports().setSwizzleId(port.get(), portId) above
              auto* depPort = helper.ports().unswizzle(dep.get<json::Helper::SwizzleId>());
              if (depPort)
              {
                port->connections().insert(depPort);
              }
              else
              {
                smtkErrorMacro(
                  smtk::io::Logger::instance(),
                  port->name() << " has unknown dependency " << dep.get<json::Helper::SwizzleId>()
                               << "; skipping.");
                continue;
              }
            }
            else if (dep.is_array() && dep.size() == 2)
            {
              // We have an array with 1 or 2 UUIDs:
              // [ null, UUID ] ⇒  dep is a component in the project resource.
              // [ UUID, null ] ⇒  dep is a resource (presumably one owned by the project).
              // [ UUID, UUID ] ⇒  dep is a component in an external resource.
              if (dep[0].is_null())
              {
                // Fetch project component:
                auto* comp = portRsrc->find(dep[1].get<smtk::common::UUID>()).get();
                if (comp)
                {
                  port->connections().insert(comp);
                }
                else
                {
                  smtkErrorMacro(
                    smtk::io::Logger::instance(),
                    "Could not find \"" << dep[1] << "\" in project resource.");
                }
              }
              else if (dep[1].is_null())
              {
                // Fetch resource.
                auto* rsrc = resourceManager->get(dep[0].get<smtk::common::UUID>()).get();
                if (rsrc)
                {
                  port->connections().insert(rsrc);
                }
                else
                {
                  smtkErrorMacro(
                    smtk::io::Logger::instance(),
                    "Could not find \"" << dep[0] << "\" in resource manager.");
                }
              }
              else
              {
                // Both dep[0] and dep[1] should be UUIDs.
                auto* rsrc = resourceManager->get(dep[0].get<smtk::common::UUID>()).get();
                if (rsrc)
                {
                  auto* comp = rsrc->find(dep[1].get<smtk::common::UUID>()).get();
                  if (comp)
                  {
                    port->connections().insert(comp);
                  }
                  else
                  {
                    smtkErrorMacro(
                      smtk::io::Logger::instance(),
                      "Could not find \"" << dep[1] << "\" in resource \"" << rsrc->name()
                                          << "\".");
                  }
                }
                else
                {
                  smtkErrorMacro(
                    smtk::io::Logger::instance(),
                    "Could not find \"" << dep[0] << "\" in resource manager.");
                }
              }
            }
          }
        }
        // TODO: UI state.
      }
    }
    // Do a second pass to deserialize dependencies and children.
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
      if (jsonTask.contains("children"))
      {
        std::set<smtk::task::TaskPtr> taskChildren;
        for (const auto& cid : jsonTask.at("children"))
        {
          if (cid.is_number_integer())
          {
            // Set by call to helper.tasks().setSwizzleId(task.get(), taskId) above
            auto* childTask = helper.tasks().unswizzle(cid.get<json::Helper::SwizzleId>());
            if (childTask)
            {
              taskChildren.insert(
                std::static_pointer_cast<smtk::task::Task>(childTask->shared_from_this()));
            }
            else
            {
              smtkErrorMacro(
                smtk::io::Logger::instance(),
                task->name() << " has unknown child task " << cid.get<json::Helper::SwizzleId>()
                             << "; skipping.");
              continue;
            }
          }
          else
          {
            auto it = taskMap.find(cid.get<smtk::common::UUID>());
            if (it == taskMap.end())
            {
              smtkErrorMacro(
                smtk::io::Logger::instance(),
                task->name() << " has unknown child " << cid << "; skipping.");
              continue;
            }
            if (it->second == task)
            {
              smtkWarningMacro(
                smtk::io::Logger::instance(), task->name() << " trying to set child to itself");
              continue;
            }
            taskChildren.insert(it->second);
          }
        }
        task->addChildren(taskChildren);
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
  (void)manager;
  // Serialize ports
  nlohmann::json::array_t portList;
  manager.portInstances().visit([&portList](const smtk::task::Port::Ptr& port) {
    // auto& helper = json::Helper::instance();
    nlohmann::json jsonPort = port;
    if (!jsonPort.is_null())
    {
      // helper.updateUIState(port, jsonTask);
      portList.push_back(jsonPort);
    }
    return smtk::common::Visit::Continue;
  });
  if (!portList.empty())
  {
    jj["ports"] = portList;
  }

  // Serialize tasks
  nlohmann::json::array_t taskList;
  manager.taskInstances().visit([&taskList](const smtk::task::Task::Ptr& task) {
    nlohmann::json jsonTask = task;
    if (!jsonTask.is_null())
    {
      taskList.push_back(jsonTask);
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
