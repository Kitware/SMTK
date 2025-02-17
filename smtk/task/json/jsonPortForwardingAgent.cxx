//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/json/jsonPortForwardingAgent.h"
#include "smtk/task/json/Helper.h"
#include "smtk/task/json/jsonTask.h"

#include "smtk/task/Manager.h"
#include "smtk/task/Port.h"

namespace smtk
{
namespace task
{

void from_json(const nlohmann::json& jj, PortForwardingAgent::ObjectFilter& objectFilter)
{
  if (jj.is_array())
  {
    objectFilter.m_resourceFilter = jj[0];
    if (jj.size() > 1)
    {
      objectFilter.m_componentFilter = jj[1];
    }
  }
  else if (jj.is_object())
  {
    objectFilter.m_resourceFilter = jj.at("resource").get<std::string>();
    if (jj.contains("component"))
    {
      objectFilter.m_componentFilter = jj.at("component").get<std::string>();
    }
  }
  else
  {
    throw std::logic_error("Wrong type of JSON data.");
  }
}

void to_json(nlohmann::json& jj, const PortForwardingAgent::ObjectFilter& objectFilter)
{
  jj = nlohmann::json::array_t{ objectFilter.m_resourceFilter, objectFilter.m_componentFilter };
}

} // namespace task
} // namespace smtk
