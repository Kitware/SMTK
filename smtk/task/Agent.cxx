//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/Agent.h"

#include "smtk/string/json/jsonToken.h"

namespace smtk
{
namespace task
{

Agent::Agent(Task* owningTask)
  : m_parent(owningTask)
{
}

std::shared_ptr<PortData> Agent::portData(const Port*) const
{
  return std::shared_ptr<PortData>();
}

void Agent::configure(const Configuration& config)
{
  auto it = config.find("name");
  if (it != config.end())
  {
    m_name = it->get<smtk::string::Token>();
  }
}

Agent::Configuration Agent::configuration() const
{
  Configuration config;
  config["type"] = this->typeName();
  if (m_name.valid())
  {
    config["name"] = m_name;
  }
  // Do not write "parent" since agent configuration is placed inside its parent's configuration.
  return config;
}

void Agent::portDataUpdated(const Port*) {}

bool Agent::getViewData(smtk::common::TypeContainer& configuration) const
{
  (void)configuration;
  return false;
}

void Agent::taskStateChanged(State, State&) {}

void Agent::taskStateChanged(Task*, State, State) {}

} // namespace task
} // namespace smtk
