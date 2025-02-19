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
    // Deserialize ports first so tasks/agents can find them upon initial
    // configuration.
    if (jj.contains("ports"))
    {
      for (const auto& jsonPort : jj.at("ports"))
      {
        helper.ports().construct(jsonPort);
      }
    }
    // Deserialize tasks; note that this does not ensure the set of
    // children or dependencies are set on each task as the order of
    // deserialization does not guarantee children/dependencies exist
    // a priori.
    if (jj.contains("tasks"))
    {
      for (const auto& jsonTask : jj.at("tasks"))
      {
        helper.tasks().construct(jsonTask);
      }
    }
    // Do a second pass on ports to deserialize connections and UI config.
    // All objects in a port's connections should now exist (tasks have
    // been created and a project should load its child resources before
    // deserializing the task manager, so external connections are also OK).
    if (jj.contains("ports"))
    {
      for (const auto& jsonPort : jj.at("ports"))
      {
        auto* port = helper.ports().get(jsonPort);
        if (!port)
        {
          smtkErrorMacro(
            smtk::io::Logger::instance(), "Could not find port for id: " << jsonPort.dump() << ".");
          continue;
        }
        if (jsonPort.contains("connections"))
        {
          auto resourceManager = helper.managers()->get<smtk::resource::Manager::Ptr>();
          const auto& jpconn(jsonPort.at("connections"));
          port->connections().reserve(jpconn.size());
          for (const auto& conn : jsonPort.at("connections"))
          {
            auto* obj = helper.objectFromJSONSpec(conn, "port");
            if (obj)
            {
              port->connections().insert(obj);
            }
            else
            {
              smtkErrorMacro(
                smtk::io::Logger::instance(),
                port->name() << " has unknown connection " << conn.dump() << "; skipping.");
              continue;
            }
          }
        }
        // TODO: UI state.
      }
    }
    // Do a second pass to deserialize task dependencies and children.
    if (jj.contains("tasks"))
    {
      for (const auto& jsonTask : jj.at("tasks"))
      {
        auto* task = helper.tasks().get(jsonTask);
        if (!task)
        {
          smtkErrorMacro(
            smtk::io::Logger::instance(), "Could not find task for id: " << jsonTask.dump() << ".");
          continue;
        }
        if (jsonTask.contains("dependencies"))
        {
          smtk::task::Task::PassedDependencies taskDeps;
          for (const auto& dep : jsonTask.at("dependencies"))
          {
            auto* depTask = helper.objectFromJSONSpecAs<smtk::task::Task>(dep, "task");
            if (!depTask)
            {
              smtkErrorMacro(
                smtk::io::Logger::instance(),
                task->name() << " has unknown dependency " << dep.dump() << "; skipping.");
              continue;
            }
            taskDeps.insert(
              std::static_pointer_cast<smtk::task::Task>(depTask->shared_from_this()));
          }
          task->addDependencies(taskDeps);
        }
        if (jsonTask.contains("children"))
        {
          std::set<smtk::task::TaskPtr> taskChildren;
          for (const auto& cid : jsonTask.at("children"))
          {
            auto* childTask = helper.objectFromJSONSpecAs<smtk::task::Task>(cid, "task");
            if (!childTask)
            {
              smtkErrorMacro(
                smtk::io::Logger::instance(),
                task->name() << " has unknown child task " << cid.dump() << "; skipping.");
              continue;
            }
            taskChildren.insert(
              std::static_pointer_cast<smtk::task::Task>(childTask->shared_from_this()));
          }
          task->addChildren(taskChildren);
        }
      }
    }

    // Now configure dependent tasks with adaptors if specified.
    // Note that tasks have already been deserialized, so the
    // helper's map from task-id to task-pointer is complete.
    if (jj.contains("adaptors"))
    {
      for (const auto& jsonAdaptor : jj.at("adaptors"))
      {
        if (!helper.adaptors().construct(jsonAdaptor))
        {
          smtkErrorMacro(
            smtk::io::Logger::instance(),
            "Could not construct adaptor for " << jsonAdaptor.dump() << "; skipping.");
        }
      }
    }

    // Copy style specifications into the task manager.
    if (jj.contains("styles"))
    {
      taskManager.setStyles(jj.at("styles"));
    }

    // Read in the worklet gallery if one exists.
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
    auto result = jj.find("toplevel-accepts-worklets-expression");
    if (result != jj.end())
    {
      if (!taskManager.toplevelExpression().setExpression(*result))
      {
        smtkErrorMacro(
          smtk::io::Logger::instance(),
          "Could not set top-level category expression to: " << *result << ".");
        taskManager.toplevelExpression().setAllPass();
      }
    }
    else
    {
      result = jj.find("toplevel-accepts-worklets");
      if (result != jj.end())
      {
        if (*result == "all")
        {
          taskManager.toplevelExpression().setAllPass();
        }
        else if (*result == "none")
        {
          taskManager.toplevelExpression().setAllReject();
        }
        else
        {
          smtkErrorMacro(
            smtk::io::Logger::instance(),
            "Found invalid toplevel-accepts-worklets: "
              << *result << " - should be set to either All or None.");
          taskManager.toplevelExpression().setAllPass();
        }
      }
      else
      {
        // There is no top-level category expression - allow all worklets to be top-level
        taskManager.toplevelExpression().setAllPass();
      }
    }
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
  if (manager.toplevelExpression().isSet())
  {
    if (manager.toplevelExpression().expression().empty())
    {
      if (manager.toplevelExpression().allPass())
      {
        jj["toplevel-accepts-worklets"] = "all";
      }
      else
      {
        jj["toplevel-accepts-worklets"] = "none";
      }
    }
    else
    {
      jj["toplevel-accepts-worklets-expression"] = manager.toplevelExpression().expression();
    }
  }
}

} // namespace task
} // namespace smtk
