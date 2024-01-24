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

Group::Group(
  const Configuration& config,
  Manager& taskManager,
  const smtk::common::Managers::Ptr& managers)
  : Task(config, taskManager, managers)
  , m_managers(managers)
{
  this->configure(config);
}

Group::Group(
  const Configuration& config,
  const PassedDependencies& dependencies,
  Manager& taskManager,
  const smtk::common::Managers::Ptr& managers)
  : Task(config, dependencies, taskManager, managers)
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
  std::vector<smtk::task::Task*> tasks;
  smtk::task::json::Helper::instance().currentTasks(tasks);
  auto& helper = smtk::task::json::Helper::pushInstance(this);
  // Instantiate children and configure them
  if (config.contains("children"))
  {
    smtk::task::from_json(config.at("children"), helper.taskManager());
    tasks.clear();
    helper.currentTasks(tasks);
    std::vector<smtk::task::Adaptor*> adaptors;
    helper.currentAdaptors(adaptors);
    for (const auto& adaptor : adaptors)
    {
      m_adaptors.push_back(adaptor->shared_from_this());
    }
    for (auto* child : tasks)
    {
      m_children.emplace(std::make_pair(
        std::static_pointer_cast<Task>(child->shared_from_this()),
        child->observers().insert(
          [this](Task& child, State prev, State next) {
            this->childStateChanged(child, prev, next);
          },
          /* priority */ 0,
          /* initialize */ true,
          "Group child observer.")));
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
        adaptor->updateDownstreamTask(adaptor->to()->state(), adaptor->to()->state());
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
          adaptor->updateDownstreamTask(adaptor->to()->state(), adaptor->to()->state());
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
