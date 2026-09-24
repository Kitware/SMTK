//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/job/Queue.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/job/Job.h"
#include "smtk/operation/Manager.h"

#include <algorithm>

namespace smtk
{
namespace job
{

Queue::Queue() = default;

Queue::Queue(const smtk::common::UUID& uid)
  : DirectSuperclass(uid)
{
}

Queue::Queue(const smtk::common::UUID& uid, resource::ManagerPtr manager)
  : DirectSuperclass(uid, manager)
{
}

Queue::Queue(resource::ManagerPtr manager)
  : DirectSuperclass(manager)
{
}

bool Queue::setId(const common::UUID& uid)
{
  if (this->Superclass::setId(uid))
  {
    // TODO: Update queue UUID in database
    return true;
  }
  return false;
}

/// A user-presentable name for the queue.
std::string Queue::name() const
{
  return m_name;
}

bool Queue::setName(const std::string& name)
{
  if (name == m_name)
  {
    return false;
  }
  m_name = name;
  return true;
}

std::string Queue::description() const
{
  return m_description;
}

bool Queue::setDescription(const std::string& description)
{
  if (m_description == description)
  {
    return false;
  }
  m_description = description;
  return true;
}

smtk::resource::ComponentPtr Queue::find(const smtk::common::UUID& compId) const
{
  return this->findJob(compId);
}

void Queue::visit(std::function<void(const smtk::resource::ComponentPtr&)>&) const {}

std::function<bool(const smtk::resource::Component&)> Queue::queryOperation(
  const std::string& query) const
{
  // TODO: Implement a query grammar.
  std::function<bool(const smtk::resource::Component&)> op =
    [query](const smtk::resource::Component& comp) {
      return query == "*" || comp.typeName() == query;
    };
  return op;
}

smtk::string::Token Queue::templateType() const
{
  static smtk::string::Token jobTemplateType("jobs");
  return jobTemplateType;
}

std::size_t Queue::templateVersion() const
{
  return 1;
}

std::unordered_set<smtk::string::Token> Queue::tags() const
{
  return m_tags;
}

bool Queue::addTag(smtk::string::Token tag)
{
  if (m_tags.find(tag) != m_tags.end())
  {
    return false;
  }
  m_tags.insert(tag);
  return true;
}

bool Queue::removeTag(smtk::string::Token tag)
{
  auto it = m_tags.find(tag);
  if (it == m_tags.end())
  {
    return false;
  }
  m_tags.erase(it);
  return true;
}

bool Queue::hasTag(smtk::string::Token tag) const
{
  return (m_tags.find(tag) != m_tags.end());
}

bool Queue::hasAllTags(const std::unordered_set<smtk::string::Token>& tagSet) const
{
  return std::all_of(tagSet.begin(), tagSet.end(), [this](const auto& tag) {
    return m_tags.find(tag) != m_tags.end();
  });
}

std::string Queue::location() const
{
  return std::string();
}

std::uint64_t Queue::maximumJobSize() const
{
  return 0;
}

std::shared_ptr<smtk::job::Job> Queue::findJob(const smtk::common::UUID&) const
{
  return std::shared_ptr<smtk::job::Job>();
}

bool Queue::add(const std::shared_ptr<Job>&)
{
  return false;
}

bool Queue::schedule(const std::shared_ptr<Job>&)
{
  return false;
}

bool Queue::cancel(const std::shared_ptr<Job>&)
{
  return false;
}

State Queue::jobState(const std::shared_ptr<Job>&)
{
  return State::Unscheduled;
}

Status Queue::jobStatus(const std::shared_ptr<Job>&)
{
  return Status::Pending;
}

std::set<std::shared_ptr<Job>> Queue::allJobs() const
{
  return std::set<std::shared_ptr<Job>>();
}

const Queue::GuardedLinks Queue::guardedLinks() const
{
  return GuardedLinks(this->mutex(), this->links());
}

Queue::GuardedLinks Queue::guardedLinks()
{
  return GuardedLinks(this->mutex(), this->links());
}

void Queue::setJobManager(smtk::job::Manager* jobManager)
{
  m_jobManager = jobManager;
}

void Queue::setOperationManager(smtk::operation::Manager::Ptr operationManager)
{
  m_operationObserverKey.release();
  m_operationManager = operationManager;
  if (operationManager)
  {
    m_operationObserverKey = operationManager->observers().insert(
      [this](
        const smtk::operation::Operation&,
        smtk::operation::EventType event,
        smtk::operation::Operation::Result result) -> int {
        if (event == smtk::operation::EventType::DID_OPERATE)
        {
          for (const auto& obj : *result->findComponent("modified"))
          {
            if (auto job = std::dynamic_pointer_cast<smtk::job::Job>(obj))
            {
              if (job->queue() == this)
              {
                // Find observers for this job and invoke them.
                auto it = m_observers.find(job->id());
                if (it != m_observers.end())
                {
                  for (const auto& entry : it->second)
                  {
                    entry.second(*job);
                  }
                }
                // Also call observers subscribed to all jobs.
                for (auto it = m_observers.begin(); it != m_observers.end() && it->first.isNull();
                     ++it)
                {
                  for (const auto& entry : it->second)
                  {
                    entry.second(*job);
                  }
                }
              }
            }
          }
          // Remove observers for jobs being removed from the queue
          // Observers do not get invoked for this as the jobs themselves
          // are not changing state.
          for (const auto& obj : *result->findComponent("expunged"))
          {
            if (auto job = std::dynamic_pointer_cast<smtk::job::Job>(obj))
            {
              auto it = m_observers.find(job->id());
              if (it != m_observers.end())
              {
                m_observers.erase(it);
              }
            }
          }
        }
        return 0; // Never cancel an operation (when event is WILL_OPERATE)
      },
      /* priority */ 0,
      /* initialize */ false,
      /* description */ "Job queue observer");
  }
}

int Queue::observe(Job* job, JobUpdateObserver observer, bool initialize)
{
  if (!observer)
  {
    return 0;
  }
  int key = m_nextObserverKey++;
  m_observers[job ? job->id() : smtk::common::UUID::null()][key] = observer;
  if (initialize)
  {
    if (job)
    {
      observer(*job);
    }
    else
    {
      for (const auto& jj : this->allJobs())
      {
        observer(*jj);
      }
    }
  }
  return key;
}

void Queue::unobserve(Job* job, int key)
{
  if (job)
  {
    auto it = m_observers.find(job->id());
    if (it != m_observers.end())
    {
      it->second.erase(key);
    }
  }
  else
  {
    for (auto& entry : m_observers)
    {
      entry.second.erase(key);
    }
  }
}

} // namespace job
} // namespace smtk
