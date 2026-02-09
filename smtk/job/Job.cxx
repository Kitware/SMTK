//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/job/Job.h"
#include "smtk/job/Queue.h"
#include "smtk/job/Resource.h"

namespace smtk
{
namespace job
{

Job::Job() {}

Job::~Job() = default;

const smtk::resource::ResourcePtr Job::resource() const
{
  return smtk::job::Resource::instance();
}

const common::UUID& smtk::job::Job::id() const
{
  return m_id;
}

bool smtk::job::Job::setId(const common::UUID& uid)
{
  if (m_id == uid || uid.isNull())
  {
    return false;
  }

  auto resource = smtk::job::Resource::instance();
  if (!resource)
  {
    m_id = uid;
    return true;
  }

  auto self = std::static_pointer_cast<Job>(this->shared_from_this());
  auto count = resource->eraseJob(self);
  smtk::common::UUID tmp = m_id;
  m_id = uid;
  // Not checking the count == 1 here. If more than one node was removed
  // that is considered an intended property of the resource NodeContainer.
  if (count > 0)
  {
    if (resource->addJob(self))
    {
      return true;
    }
    else
    {
      m_id = tmp;
      return false;
    }
  }

  m_id = uid;
  return true;
}

Queue* Job::queue() const
{
  return m_queue;
}

bool Job::setQueue(Queue* qq)
{
  if (m_queue == qq || m_queue)
  {
    // If we already have a queue, we cannot reparent the job.
    // Its script may make queue-specific assumptions (such as
    // the launcher type).
    return false;
  }
  m_queue = qq;
  return true;
}

std::uint64_t Job::size() const
{
  return m_size;
}

bool Job::setSize(std::uint64_t jobSize)
{
  if (jobSize == m_size)
  {
    return false;
  }
  m_size = jobSize;
  return true;
}

std::filesystem::path Job::caseDirectory() const
{
  return m_caseDirectory;
}

bool Job::setCaseDirectory(std::filesystem::path dir)
{
  if (dir == m_caseDirectory || dir.empty())
  {
    return false;
  }
  m_caseDirectory = dir;
  return true;
}

std::filesystem::path Job::script() const
{
  return m_script;
}

bool Job::setScript(std::filesystem::path scriptPath)
{
  if (scriptPath.empty() || scriptPath == m_script)
  {
    return false;
  }
  m_script = scriptPath;
  return true;
}

const std::vector<std::filesystem::path>& Job::logs() const
{
  return m_logs;
}

bool Job::setLogs(const std::vector<std::filesystem::path>& logFiles)
{
  if (m_logs == logFiles)
  {
    return false;
  }
  m_logs = logFiles;
  return true;
}

std::string Job::queueId() const
{
  return m_queueId;
}

bool Job::setQueueId(const std::string& queueId)
{
  if (queueId == m_queueId)
  {
    return false;
  }

  m_queueId = queueId;
  return true;
}

bool Job::schedule(Queue* queue)
{
  if (queue)
  {
    m_queue = queue;
    return queue->schedule(shared_from_this());
  }
  else if (m_queue)
  {
    return m_queue->schedule(shared_from_this());
  }
  return false;
}

State Job::state() const
{
  auto self = const_cast<Job*>(this)->shared_from_this();
  return m_queue ? m_queue->jobState(self) : State::Unscheduled;
}

Status Job::status() const
{
  auto self = const_cast<Job*>(this)->shared_from_this();
  return m_queue ? m_queue->jobStatus(self) : Status::Pending;
}

bool Job::setAutoSchedule(bool shouldSchedule)
{
  if (m_autoSchedule == shouldSchedule)
  {
    return false;
  }
  m_autoSchedule = shouldSchedule;
  return true;
}

const Job::LogParserMap& Job::logParsers() const
{
  return m_logParsers;
}

bool Job::setLogParser(smtk::string::Token logPath, const LogParser& parser)
{
  (void)logPath;
  (void)parser;
  return false;
}

bool Job::clearLogParser(smtk::string::Token logPath)
{
  return false;
}

bool Job::resetLogParsers()
{
  return false;
}

Job::LinkKey Job::linkTo(const std::shared_ptr<PersistentObject>& object)
{
  if (!object)
  {
    return LinkKey();
  }

  // If the object is a component...
  if (auto component = std::dynamic_pointer_cast<smtk::resource::Component>(object))
  {
    return this->guardedLinks()->addLinkTo(component, smtk::job::Resource::jobOriginRole());
  }
  // If the object is a resource...
  else if (auto resource = std::dynamic_pointer_cast<smtk::resource::Resource>(object))
  {
    return this->guardedLinks()->addLinkTo(resource, smtk::job::Resource::jobOriginRole());
  }

  // If the object cannot be cast to a resource or component, there's not much
  // we can do.
  return LinkKey();
}

bool Job::unlink(LinkKey key)
{
  return this->guardedLinks()->removeLink(key);
}

std::set<std::shared_ptr<smtk::resource::PersistentObject>> Job::originators() const
{
  return this->guardedLinks()->linkedTo(smtk::job::Resource::jobOriginRole());
}

const Job::GuardedLinks Job::guardedLinks() const
{
  return GuardedLinks(Resource::instance()->mutex(), this->links());
}

Job::GuardedLinks Job::guardedLinks()
{
  return GuardedLinks(Resource::instance()->mutex(), this->links());
}

bool Job::setContainerImage(const std::string& imageURL)
{
  if (imageURL == m_containerImage)
  {
    return false;
  }
  m_containerImage = imageURL;
  return true;
}

bool Job::setCaseDirectoryMountPoint(const std::string& mountPoint)
{
  if (mountPoint == m_caseDirectoryMountPoint)
  {
    return false;
  }
  m_caseDirectoryMountPoint = mountPoint;
  return true;
}

} // namespace job
} // namespace smtk
