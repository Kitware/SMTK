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

#include "smtk/job/Definition.h"
#include "smtk/job/Queue.h"
// #include "smtk/job/Resource.h"
#include "smtk/job/Stage.h"

namespace smtk
{
namespace job
{

Job::Job() {}

Job::~Job() = default;

const smtk::resource::ResourcePtr Job::resource() const
{
  return m_queue ? m_queue->shared_from_this() : smtk::resource::ResourcePtr();
}

bool Job::setJobType(const std::shared_ptr<Definition>& jobType)
{
  return this->setJobType(jobType.get());
}

bool Job::setJobType(Definition* jobType)
{
  // The previous job type must be invalid and the new value must be valid:
  if (m_jobType || !jobType)
  {
    return false;
  }
  m_jobType = jobType;
  // TODO: Synchronize w/ storage.
  return true;
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

#if 0
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
#endif

  m_id = uid;
  return true;
}

std::string Job::name() const
{
  if (m_queueId.empty())
  {
    return this->Superclass::name();
  }
  return m_queueId;
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
  // TODO: Synchronize w/ storage.
  return true;
}

std::filesystem::path Job::script() const
{
  return m_jobType ? m_jobType->script() : std::filesystem::path();
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
  // TODO: Synchronize w/ storage.
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

bool Job::setState(State s)
{
  if (m_state == s)
  {
    return false;
  }

  m_state = s;
  return true;
}

bool Job::setStatus(Status s)
{
  if (m_status == s)
  {
    return false;
  }

  m_status = s;
  return true;
}

bool Job::setStage(int s)
{
  if (m_stage == s)
  {
    return false;
  }

  m_stage = s;
  return true;
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

Job::LinkKey Job::linkTo(const std::shared_ptr<PersistentObject>& object)
{
  if (!object)
  {
    return LinkKey();
  }

  // If the object is a component...
  if (auto component = std::dynamic_pointer_cast<smtk::resource::Component>(object))
  {
    return this->guardedLinks()->addLinkTo(component, smtk::job::Queue::jobOriginRole());
  }
  // If the object is a resource...
  else if (auto resource = std::dynamic_pointer_cast<smtk::resource::Resource>(object))
  {
    return this->guardedLinks()->addLinkTo(resource, smtk::job::Queue::jobOriginRole());
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
  return this->guardedLinks()->linkedTo(smtk::job::Queue::jobOriginRole());
}

const Job::GuardedLinks Job::guardedLinks() const
{
  return GuardedLinks(m_queue->mutex(), this->links());
}

Job::GuardedLinks Job::guardedLinks()
{
  return GuardedLinks(m_queue->mutex(), this->links());
}

std::string Job::containerImage() const
{
  return m_containerImage.empty() ? m_jobType->containerImage() : m_containerImage;
}

bool Job::setContainerImage(const std::string& imageURL)
{
  if (imageURL == m_containerImage)
  {
    return false;
  }
  if (imageURL == m_jobType->containerImage())
  {
    // If the new URL is the default container image of the job definition,
    // then clear out the local ivar.
    bool didModify = !m_containerImage.empty();
    m_containerImage.clear();
    // TODO: Synchronize w/ storage.
    return didModify;
  }
  m_containerImage = imageURL;
  // TODO: Synchronize w/ storage.
  return true;
}

std::string Job::caseDirectoryMountPoint() const
{
  return m_caseDirectoryMountPoint.empty() ? m_jobType->caseDirectoryMountPoint()
                                           : m_caseDirectoryMountPoint;
}

bool Job::setCaseDirectoryMountPoint(const std::string& mountPoint)
{
  if (mountPoint == m_caseDirectoryMountPoint)
  {
    return false;
  }
  if (mountPoint == m_jobType->caseDirectoryMountPoint())
  {
    // If the new mount point is the default mount point of the job definition,
    // then clear out the local ivar.
    bool didModify = !m_caseDirectoryMountPoint.empty();
    m_caseDirectoryMountPoint.clear();
    // TODO: Synchronize w/ storage.
    return didModify;
  }
  // TODO: Synchronize w/ storage.
  m_caseDirectoryMountPoint = mountPoint;
  return true;
}

LogParser* Job::logParser(smtk::string::Token logPath) const
{
  auto it = m_logParsers.find(logPath);
  if (it == m_logParsers.end())
  {
    return nullptr;
  }
  return it->second.get();
}

bool Job::restore(
  smtk::job::Queue* queue,
  const smtk::common::UUID& uid,
  Definition* jobType,
  std::uint64_t size,
  smtk::job::State state,
  smtk::job::Status status,
  int stage,
  const std::string& queueId,
  const std::string& caseDirectory,
  const std::string& containerImageUrl,
  const std::string& mountPoint,
  bool autoSchedule)
{
  // Do not allow a job to be restored from a mismatched ID or different queue.
  if (
    !queue || (m_queue && m_queue != queue) || uid.isNull() ||
    (!this->id().isNull() && this->id() != uid))
  {
    return false;
  }
  m_queue = queue;
  m_id = uid;
  m_jobType = jobType;
  m_size = size;
  m_queueId = queueId;
  m_state = state;
  m_status = status;
  m_stage = stage;
  m_caseDirectory = caseDirectory;
  m_containerImage = containerImageUrl;
  m_caseDirectoryMountPoint = mountPoint;
  m_autoSchedule = autoSchedule;
  return true;
}

} // namespace job
} // namespace smtk
