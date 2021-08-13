//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/Group.h"

#include "smtk/project/ResourceContainer.h"

#include "smtk/task/json/Helper.h"
#include "smtk/task/json/jsonGroup.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/SpecificationOps.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Resource.h"

#include "smtk/resource/Manager.h"

#include <stdexcept>

namespace smtk
{
namespace task
{

Group::Group() = default;

Group::Group(const Configuration& config, const smtk::common::Managers::Ptr& managers)
  : Task(config, managers)
  , m_managers(managers)
{
  this->configure(config);
}

Group::Group(
  const Configuration& config,
  const PassedDependencies& dependencies,
  const smtk::common::Managers::Ptr& managers)
  : Task(config, dependencies, managers)
  , m_managers(managers)
{
  this->configure(config);
}

void Group::configure(const Configuration& config)
{
  // Instantiate children and configure them
}

std::vector<Task::Ptr> Group::children() const
{
  std::vector<Task::Ptr> result;
  result.reserve(m_children.size());
  for (const auto& entry : m_children)
  {
    result.push_back(entry.first);
  }
  return result;
}

State Group::computeInternalState() const
{
  State s = State::Completable;
  for (const auto& entry : m_children)
  {
    auto childState = entry.first->state();
    if (childState < s)
    {
      s = childState;
    }
  }
  return s;
}

} // namespace task
} // namespace smtk
