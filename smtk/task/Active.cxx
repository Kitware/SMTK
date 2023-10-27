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
#include "smtk/task/Instances.h"
#include "smtk/task/Manager.h"
#include "smtk/task/Task.h"

namespace smtk
{
namespace task
{

constexpr const char* const Active::type_name;

struct Active::Internal
{
  Internal(Active* self, smtk::task::Instances* instances)
    : m_self(self)
    , m_instances(instances)
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

  smtk::task::Active* m_self;
  smtk::task::Instances* m_instances;
  smtk::task::Instances::Observers::Key m_instancesObserver;
  std::weak_ptr<smtk::task::Task> m_active;
  smtk::task::Task::Observers::Key m_activeObserver;
  Observers m_observers;
};

Active::Active(smtk::task::Instances* instances)
  : m_p(new Internal(this, instances))
{
}

Active::~Active()
{
  delete m_p;
  m_p = nullptr;
}

smtk::task::Task* Active::task() const
{
  return m_p->m_active.lock().get();
}

bool Active::switchTo(smtk::task::Task* task)
{
  auto current = m_p->m_active.lock();
  if (current.get() == task)
  {
    return false;
  }
  if (!task)
  {
    if (current)
    {
      m_p->m_observers(current.get(), nullptr);
      m_p->m_active.reset();
      m_p->m_activeObserver.release();
      return true;
    }
    return false;
  }
  if (task->state() == State::Unavailable)
  {
    return false;
  }
  auto sharedTask = std::static_pointer_cast<Task>(task->shared_from_this());
  if (m_p->m_instances && !m_p->m_instances->contains(sharedTask))
  {
    // Only managed tasks can be active if we have instances tracked.
    return false;
  }
  m_p->m_observers(current.get(), task);
  m_p->m_active = sharedTask;
  m_p->m_activeObserver = task->observers().insert([this](Task&, State, State next) {
    if (next == State::Unavailable)
    {
      this->switchTo(nullptr);
    }
  });
  return true;
}

Active::Observers& Active::observers()
{
  return m_p->m_observers;
}

const Active::Observers& Active::observers() const
{
  return m_p->m_observers;
}

} // namespace task
} // namespace smtk
