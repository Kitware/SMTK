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

namespace smtk
{
namespace job
{

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

std::string Queue::location() const
{
  return std::string();
}

std::uint64_t Queue::maximumJobSize() const
{
  return 0;
}

bool Queue::schedule(const std::shared_ptr<Job>& job)
{
  return false;
}

bool Queue::cancel(const std::shared_ptr<Job>& job)
{
  return false;
}

State Queue::jobState(const std::shared_ptr<Job>& job)
{
  (void)job;
  return State::Unscheduled;
}

Status Queue::jobStatus(const std::shared_ptr<Job>& job)
{
  (void)job;
  return Status::Pending;
}

std::set<std::shared_ptr<Job>> Queue::allJobs() const
{
  return std::set<std::shared_ptr<Job>>();
}

} // namespace job
} // namespace smtk
