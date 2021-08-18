//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/json/jsonGroup.h"
#include "smtk/task/json/Helper.h"
#include "smtk/task/json/jsonTask.h"

#include "smtk/task/Group.h"

namespace smtk
{
namespace task
{
namespace json
{

Task::Configuration jsonGroup::operator()(const Task* task, Helper& helper) const
{
  Task::Configuration config;
  auto* nctask = const_cast<Task*>(task);
  auto* group = dynamic_cast<Group*>(nctask);
  if (group)
  {
    jsonTask superclass;
    config = superclass(group, helper);
    nlohmann::json::array_t children;
    for (const auto& child : group->children())
    {
      nlohmann::json jsonChild = child;
      children.push_back(jsonChild);
    }
    config["children"] = children;
  }
  return config;
}

} // namespace json
} // namespace task
} // namespace smtk
