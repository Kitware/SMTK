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
#include "smtk/task/json/jsonManager.h"

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
  if (config.contains("adaptor-data"))
  {
    m_adaptorData = config.at("adaptor-data");
  }
  // Push a new helper onto the stack (so task/adaptor numbers
  // refer to children, not exernal objects):
  auto& helper = smtk::task::json::Helper::pushInstance(this);
  // Instantiate children and configure them
  if (config.contains("children"))
  {
    std::vector<std::weak_ptr<smtk::task::Task>> tasks;
    if (!smtk::task::json::jsonManager::deserialize(
          tasks, m_adaptors, helper.managers(), config.at("children")))
    {
      smtkErrorMacro(smtk::io::Logger::instance(), "Could not deserialize child tasks.");
    }
    for (const auto& weakChild : tasks)
    {
      if (auto child = weakChild.lock())
      {
        m_children.emplace(std::make_pair(
          child,
          child->observers().insert(
            [this](Task& child, State prev, State next) {
              this->childStateChanged(child, prev, next);
            },
            /* priority */ 0,
            /* initialize */ true,
            "Group child observer.")));
      }
    }
  }
  else
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "No children tasks, group will be trivial.");
  }
  // Revert helper instance to external objects:
  smtk::task::json::Helper::popInstance();

  this->internalStateChanged(this->computeInternalState());
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

smtk::common::Visit Group::visit(RelatedTasks relation, Visitor visitor) const
{
  switch (relation)
  {
    case RelatedTasks::Depend:
      return this->Superclass::visit(relation, visitor);
      break;
    case RelatedTasks::Child:
      for (const auto& entry : m_children)
      {
        auto dep = entry.first;
        if (dep)
        {
          if (visitor(*dep) == smtk::common::Visit::Halt)
          {
            return smtk::common::Visit::Halt;
          }
        }
      }
      break;
  }
  return smtk::common::Visit::Continue;
}

void Group::setAdaptorData(const std::string& tagName, Task::Configuration& config)
{
  auto it = m_adaptorData.find(tagName);
  if (it != m_adaptorData.end())
  {
    if (m_adaptorData[tagName] == config)
    { // Data is identical to prior data.
      return;
    }
  }
  // Now we know we have new data.
  // Assign it and reconfigure children.
  m_adaptorData[tagName] = config;
  for (const auto& weakAdaptor : m_adaptors)
  {
    if (auto adaptor = weakAdaptor.lock())
    {
      if (adaptor->from() == this)
      {
        adaptor->reconfigureTask();
      }
    }
  }
}

Task::Configuration Group::adaptorData(const std::string& key) const
{
  if (m_adaptorData.contains(key))
  {
    return m_adaptorData.at(key);
  }
  return {};
}

State Group::computeInternalState() const
{
  bool allIrrelevant = true;
  bool allUnavailable = true;
  State s = State::Completable;
  for (const auto& entry : m_children)
  {
    auto childState = entry.first->state();
    if (childState != State::Irrelevant)
    {
      allIrrelevant = false;
    }
    if (childState != State::Unavailable)
    {
      allUnavailable = false;
    }
    if (childState > State::Unavailable && childState < s)
    {
      s = childState;
    }
  }
  if (!m_children.empty())
  {
    if (allIrrelevant)
    {
      s = State::Irrelevant;
    }
    else if (allUnavailable)
    {
      s = State::Unavailable;
    }
  }
  // If the internal state is about to change to completable,
  // ensure any output adaptor-data is updated.
  if (s > this->internalState() && s >= State::Completable)
  {
    for (const auto& weakAdaptor : m_adaptors)
    {
      if (auto adaptor = weakAdaptor.lock())
      {
        if (adaptor->to() == this)
        {
          adaptor->reconfigureTask();
        }
      }
    }
  }
  return s;
}

void Group::childStateChanged(Task& child, State prev, State next)
{
  if (prev == next)
  {
    return;
  }
  // For now, bludgeon internal state into submission by recomputing
  // from scratch. In the future we should do this incrementally.
  // TODO
  this->internalStateChanged(this->computeInternalState());
  (void)child;
  // (void)prev;
  // (void)next;
}

} // namespace task
} // namespace smtk
