//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/json/jsonTask.h"

#include "smtk/task/json/Helper.h"
#include "smtk/task/json/jsonAgent.h"
#include "smtk/task/json/jsonFillOutAttributesAgent.h"
#include "smtk/task/json/jsonSubmitOperationAgent.h"

#include "smtk/task/Manager.h"

#include "smtk/string/json/jsonManager.h"
#include "smtk/string/json/jsonToken.h"

#include "smtk/io/Logger.h"

namespace smtk
{
namespace task
{
namespace json
{

Task::Configuration jsonTask::operator()(const Task* task, Helper& helper) const
{
  Task::Configuration config;
  if (task)
  {
    config["id"] = task->id();
    config["swizzle"] = helper.tasks().swizzleId(task);
    config["type"] = task->typeName();
    config["name"] = task->name();
    if (!task->description().empty())
    {
      config["description"] = task->description();
    }
    if (!task->style().empty())
    {
      config["style"] = task->style();
    }
    config["agentState"] = stateName(task->agentState());
    config["childrenState"] = stateName(task->childrenState());
    config["dependencyState"] = stateName(task->dependencyState());
    config["completed"] = task->isCompleted();
    nlohmann::json::array_t deps;
    for (const auto& dependency : task->dependencies())
    {
      deps.push_back(dependency->id());
    }
    if (!deps.empty())
    {
      config["dependencies"] = deps;
    }
    config["strict-dependencies"] = task->areDependenciesStrict();
    nlohmann::json portDict;
    for (const auto& entry : task->ports())
    {
      if (entry.second)
      {
        portDict[entry.first.data()] = entry.second->id();
      }
    }
    if (!portDict.empty())
    {
      config["ports"] = portDict;
    }

    nlohmann::json::array_t agentArr;
    for (const auto& agent : task->agents())
    {
      if (agent)
      {
        agentArr.push_back(agent->configuration());
      }
    }
    if (!agentArr.empty())
    {
      config["agents"] = agentArr;
    }

    nlohmann::json::array_t childArr;
    for (const auto& child : task->children())
    {
      if (child)
      {
        childArr.emplace_back(child->id());
      }
    }
    if (!childArr.empty())
    {
      config["children"] = childArr;
    }
  }
  return config;
}

} // namespace json

void to_json(nlohmann::json& jj, const smtk::task::Task::Ptr& task)
{
  if (!task)
  {
    return;
  }
  auto& helper = json::Helper::instance();
  jj = helper.tasks().configuration(task.get());
  if (helper.taskManager().active().task() == task.get())
  {
    jj["active"] = true;
  }
}

void from_json(const nlohmann::json& jj, smtk::task::Task::Ptr& task)
{
  try
  {
    auto& helper = json::Helper::instance();
    auto managers = helper.managers();
    auto& taskManager = helper.taskManager();
    auto taskType = jj.contains("type") ? jj.at("type").get<std::string>() : "smtk::task::Task";
    task = taskManager.taskInstances().createFromName(
      taskType, const_cast<nlohmann::json&>(jj), managers);
    if (jj.contains("active") && jj.at("active").get<bool>())
    {
      helper.setActiveSerializedTask(task.get());
    }
  }
  catch (nlohmann::json::exception& e)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Could not deserialize task (" << e.what() << ").");
  }
}

} // namespace task
} // namespace smtk
