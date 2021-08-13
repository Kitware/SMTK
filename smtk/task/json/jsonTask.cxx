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

#include "smtk/task/Manager.h"

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
    config["id"] = helper.tasks().swizzleId(task);
    config["type"] = task->typeName();
    config["title"] = task->title();
    config["state"] = stateName(task->internalState());
    auto deps = helper.swizzleDependencies(task->dependencies());
    if (!deps.empty())
    {
      config["dependencies"] = deps;
    }
  }
  return config;
}

} // namespace json

void to_json(nlohmann::json& j, const smtk::task::Task::Ptr& task)
{
  if (!task)
  {
    return;
  }
  auto& helper = json::Helper::instance();
  j = helper.tasks().configuration(task.get());
}

void from_json(const nlohmann::json& j, smtk::task::Task::Ptr& task)
{
  try
  {
    auto& helper = json::Helper::instance();
    auto managers = helper.managers();
    auto taskManager = managers->get<std::shared_ptr<smtk::task::Manager>>();
    auto taskType = j.at("type").get<std::string>();
    task = taskManager->taskInstances().createFromName(
      taskType, const_cast<nlohmann::json&>(j), managers);
  }
  catch (std::exception& e)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Could not deserialize task (" << e.what() << ").");
  }
}

} // namespace task
} // namespace smtk
