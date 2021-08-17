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
#include "smtk/task/json/jsonAdaptor.h"
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
    config["adaptor-data"] = group->adaptorData();
    // Now that we've serialized the parent task,
    // push a helper on the stack to serialize children.
    auto& childHelper = smtk::task::json::Helper::pushInstance(group);
    nlohmann::json::array_t children;
    for (const auto& child : group->children())
    {
      childHelper.tasks().swizzleId(child.get());
      nlohmann::json jsonChild = child;
      children.emplace_back(jsonChild);
    }
    nlohmann::json::array_t adaptors;
    for (const auto& weakAdaptor : group->adaptors())
    {
      auto adaptor = weakAdaptor.lock();
      if (adaptor)
      {
        childHelper.adaptors().swizzleId(adaptor.get());
        nlohmann::json jsonAdaptor = adaptor;
        adaptors.emplace_back(jsonAdaptor);
      }
    }
    config["children"] = { { "tasks", children }, { "adaptors", adaptors } };
    smtk::task::json::Helper::popInstance();
  }
  return config;
}

} // namespace json
} // namespace task
} // namespace smtk
