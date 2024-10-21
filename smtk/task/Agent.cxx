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

Agent::Configuration Agent::configuration() const
{
  Configuration config;
  config["type"] = this->typeName();
  // Do not write "parent" since agent configuration is placed inside its parent's configuration.
  return config;
}

void Agent::portDataUpdated(const Port*) {}

void Agent::taskStateChanged(State, State&) {}

void Agent::taskStateChanged(Task*, State, State) {}

} // namespace task
} // namespace smtk
