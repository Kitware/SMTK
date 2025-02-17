//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/json/jsonObjectsInRoles.h"

#include "smtk/common/Managers.h"
#include "smtk/resource/Manager.h"
#include "smtk/task/json/Helper.h"

namespace smtk
{
namespace task
{

void from_json(const nlohmann::json& jj, std::shared_ptr<ObjectsInRoles>& objectsInRoles)
{
  if (objectsInRoles)
  {
    objectsInRoles->clear();
  }
  else
  {
    objectsInRoles = std::make_shared<smtk::task::ObjectsInRoles>();
  }

  auto& helper(smtk::task::json::Helper::instance());
  // auto& taskManager(helper.taskManager());
  // auto& mgrs = helper.managers();

  for (const auto& entry : jj.items())
  {
    smtk::string::Token role(entry.key());
    if (entry.value().is_array())
    {
      for (const auto& objectSpec : entry.value())
      {
        auto* obj = helper.objectFromJSONSpec(objectSpec, "port");
        if (obj)
        {
          objectsInRoles->addObject(obj, role);
        }
      }
    }
  }
}

void to_json(nlohmann::json& jj, const std::shared_ptr<ObjectsInRoles>& objectsInRoles)
{
  if (!objectsInRoles)
  {
    return;
  }

  for (const auto& entry : objectsInRoles->data())
  {
    nlohmann::json::array_t objectSet;
    for (const auto& object : entry.second)
    {
      if (auto* rsrc = dynamic_cast<smtk::resource::Resource*>(object))
      {
        objectSet.push_back({ rsrc->id(), nullptr });
      }
      else if (auto* comp = dynamic_cast<smtk::resource::Component*>(object))
      {
        objectSet.push_back({ comp->parentResource()->id(), comp->id() });
      }
    }
    if (!objectSet.empty())
    {
      jj[entry.first.data()] = objectSet;
    }
  }
}

} // namespace task
} // namespace smtk
