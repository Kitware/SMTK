//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/Active.h"

namespace smtk
{
namespace task
{

Active::Active(smtk::task::Instances* instances)
  : m_instances(instances)
  , m_observers(/* initializer */
                [this](Observer& observer) {
                  auto active = m_active.lock();
                  // Ony initialization will ever call an observer with the same value for both parameters.
                  observer(active.get(), active.get());
                })
{
  if (m_instances)
  {
    m_instancesObserver = m_instances->observers().insert(
      [this](smtk::common::InstanceEvent event, const std::shared_ptr<smtk::task::Task>& task) {
        if (event == smtk::common::InstanceEvent::Unmanaged && task && task == m_active.lock())
        {
          m_observers(task.get(), nullptr);
          m_active.reset();
        }
      },
      /* priority */ 0,
      /* initialize */ false,
      "Observe task deletion for task::Active");
  }
}

Active::~Active() = default;

smtk::task::Task* Active::task() const
{
  return m_active.lock().get();
}

bool Active::switchTo(smtk::task::Task* task)
{
  auto current = m_active.lock();
  if (current.get() == task)
  {
    return false;
  }
  if (!task)
  {
    if (current)
    {
      m_observers(current.get(), nullptr);
      m_active.reset();
      m_activeObserver.release();
      return true;
    }
    return false;
  }
  if (task->state() == State::Unavailable)
  {
    return false;
  }
  auto sharedTask = task->shared_from_this();
  if (m_instances && !m_instances->contains(sharedTask))
  {
    // Only managed tasks can be active if we have instances tracked.
    return false;
  }
  m_observers(current.get(), task);
  m_active = sharedTask;
  m_activeObserver = task->observers().insert([this](Task&, State, State next) {
    if (next == State::Unavailable)
    {
      this->switchTo(nullptr);
    }
  });
  return true;
}

} // namespace task
} // namespace smtk
