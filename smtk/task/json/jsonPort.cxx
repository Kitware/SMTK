//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/json/jsonPort.h"
#include "smtk/task/json/Helper.h"

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

Port::Configuration jsonPort::operator()(const Port* port, Helper& helper) const
{
  Port::Configuration config;
  if (port)
  {
    config["id"] = port->id();
    config["swizzle"] = helper.ports().swizzleId(port);
    config["type"] = port->typeName();
    config["name"] = port->name();
    config["direction"] = Port::LabelFromDirection(port->direction());
    config["data-types"] = port->dataTypes();
    if (!port->style().empty())
    {
      config["style"] = port->style();
    }
    if (port->parent())
    {
      config["parent"] = port->parent()->id();
    }
    nlohmann::json::array_t jconn;
    jconn.reserve(port->connections().size());
    auto* portRsrc = port->parentResource();
    for (const auto& conn : port->connections())
    {
      if (!conn)
      {
        continue;
      }
      if (auto* comp = dynamic_cast<smtk::resource::Component*>(conn))
      {
        auto* rsrc = comp->parentResource();
        if (rsrc == portRsrc)
        {
          jconn.push_back({ nullptr, comp->id() });
        }
        else
        {
          jconn.push_back({ rsrc->id(), comp->id() });
        }
      }
      else if (auto* rsrc = dynamic_cast<smtk::resource::Resource*>(conn))
      {
        jconn.push_back({ rsrc->id(), nullptr });
      }
      else
      {
        smtkWarningMacro(
          smtk::io::Logger::instance(),
          "Unknown object " << conn << " of type \"" << conn->typeName()
                            << "\""
                               " serving as port connection. Skipping.");
      }
    }
    if (!jconn.empty())
    {
      config["connections"] = jconn;
    }
  }
  return config;
}

} // namespace json

void to_json(nlohmann::json& jj, const smtk::task::Port::Ptr& port)
{
  if (!port)
  {
    return;
  }
  auto& helper = json::Helper::instance();
  jj = helper.ports().configuration(port.get());
}

void from_json(const nlohmann::json& jj, smtk::task::Port::Ptr& port)
{
  try
  {
    auto& helper = json::Helper::instance();
    auto managers = helper.managers();
    auto& taskManager = helper.taskManager();
    auto portType = jj.at("type").get<std::string>();
    auto pit = jj.find("parent");
    Task* parentTask = nullptr;
    if (pit != jj.end())
    {
      if (pit->is_number_integer())
      {
        auto taskSwizzle = pit->get<json::Helper::SwizzleId>();
        parentTask = helper.tasks().unswizzle(taskSwizzle);
      }
      else
      {
        auto taskId = pit->get<smtk::common::UUID>();
        parentTask = taskManager.taskInstances().findById(taskId).get();
      }
      if (parentTask)
      {
        port = taskManager.portInstances().createFromName(
          portType, const_cast<nlohmann::json&>(jj), parentTask, managers);
      }
    }
    if (!port)
    {
      port = taskManager.portInstances().createFromName(
        portType, const_cast<nlohmann::json&>(jj), managers);
    }
  }
  catch (nlohmann::json::exception& e)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Could not deserialize port (" << e.what() << ").");
  }
}

} // namespace task
} // namespace smtk
