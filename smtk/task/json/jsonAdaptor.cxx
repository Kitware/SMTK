//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/json/jsonAdaptor.h"
#include "smtk/task/json/Helper.h"

#include "smtk/task/Manager.h"

#include "smtk/io/Logger.h"

namespace smtk
{
namespace task
{
namespace json
{

Adaptor::Configuration jsonAdaptor::operator()(const Adaptor* adaptor, Helper& helper) const
{
  Adaptor::Configuration config;
  if (adaptor)
  {
    config = { { "id", helper.adaptors().swizzleId(adaptor) },
               { "type", adaptor->typeName() },
               { "from", helper.tasks().swizzleId(adaptor->from()) },
               { "to", helper.tasks().swizzleId(adaptor->to()) } };
  }
  return config;
}

} // namespace json

void to_json(nlohmann::json& j, const smtk::task::Adaptor::Ptr& adaptor)
{
  if (!adaptor)
  {
    return;
  }
  auto& helper = json::Helper::instance();
  j = helper.adaptors().configuration(adaptor.get());
}

void from_json(const nlohmann::json& j, smtk::task::Adaptor::Ptr& adaptor)
{
  try
  {
    auto& helper = json::Helper::instance();
    auto managers = helper.managers();
    auto& taskManager = helper.taskManager();
    auto adaptorType = j.at("type").get<std::string>();
    auto taskPair = helper.getAdaptorTasks();
    adaptor = taskManager.adaptorInstances().createFromName(
      adaptorType, const_cast<nlohmann::json&>(j), taskPair.first, taskPair.second);
  }
  catch (std::exception& e)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Could not deserialize adaptor (" << e.what() << ").");
  }
}

} // namespace task
} // namespace smtk
