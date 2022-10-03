//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/resource/Lock.h"

#include "smtk/resource/Resource.h"

namespace smtk
{
namespace resource
{

Lock::Lock() = default;

void Lock::lock(LockType lockType)
{
  if (lockType == LockType::Read)
  {
    // Lock the resource.
    std::unique_lock<std::mutex> lk(m_mutex);

    // Wait for writers to finish.
    while (m_waitingWriters != 0)
    {
      m_readerCondition.wait(lk);
    }

    // Declare yourself as a reader.
    ++m_activeReaders;

    // Unlock the resource.
    lk.unlock();
  }
  else if (lockType == LockType::Write)
  {
    // Lock the resource.
    std::unique_lock<std::mutex> lk(m_mutex);

    // Declare yourself as a waiting writier.
    ++m_waitingWriters;

    // Wait for active readers and active writers to finish.
    while (m_activeReaders != 0 || m_activeWriters != 0)
    {
      m_writerCondition.wait(lk);
    }

    // Declare yourself as an active writer.
    ++m_activeWriters;

    // Unlock the resource.
    lk.unlock();
  }
}

bool Lock::tryLock(LockType lockType)
{
  std::unique_lock<std::mutex> lk(m_mutex);
  if (lockType == LockType::Read)
  {
    // Writers are blocking acquisition.
    if (m_waitingWriters != 0)
    {
      lk.unlock();
      return false;
    }
    // Declare this callers as a reader.
    ++m_activeReaders;
    lk.unlock();
    return true;
  }
  else if (lockType == LockType::Write)
  {
    if (m_activeReaders != 0 || m_activeWriters != 0)
    {
      lk.unlock();
      return false;
    }
    // Declare the caller as *the* writer.
    // Note that the active writer also always marks itself as waiting until unlocked.
    ++m_waitingWriters;
    ++m_activeWriters;
    lk.unlock();
    return true;
  }
  // Unlock the resource.
  lk.unlock();
  return false;
}

void Lock::unlock(LockType lockType)
{
  if (lockType == LockType::Read)
  {
    // Lock the resource.
    std::unique_lock<std::mutex> lk(m_mutex);

    // Remove yourself as an active reader.
    --m_activeReaders;

    // Unlock the resource.
    lk.unlock();

    // Tell one of the waiting writers to check if it can write.
    m_writerCondition.notify_one();
  }
  else if (lockType == LockType::Write)
  {
    // Lock the resource.
    std::unique_lock<std::mutex> lk(m_mutex);

    // Remove yourself as a waiting writer.
    --m_waitingWriters;

    // Remove yourself as an active writer.
    --m_activeWriters;

    if (m_waitingWriters > 0)
    {
      // If there are writers waiting to write, tell one of them to check if it
      // can write.
      m_writerCondition.notify_one();
    }
    else
    {
      // Otherwise, tell all readers that they can read.
      m_readerCondition.notify_all();
    }

    // Unlock the resource.
    lk.unlock();
  }
}

smtk::resource::LockType Lock::state() const
{
  return (
    m_activeWriters > 0 ? LockType::Write
                        : (m_activeReaders > 0 ? LockType::Read : LockType::Unlocked));
}

ScopedLockGuard::ScopedLockGuard(Lock& lock, LockType lockType)
  : m_lock(lock)
  , m_lockType(lockType)
{
  m_lock.lock(m_lockType);
}

ScopedLockGuard::ScopedLockGuard(Lock& alreadyAcquiredLock, LockType lockType, bool alreadyAcquired)
  : m_lock(alreadyAcquiredLock)
  , m_lockType(lockType)
{
  (void)alreadyAcquired;
}

ScopedLockGuard::~ScopedLockGuard()
{
  m_lock.unlock(m_lockType);
}

bool ScopedLockGuard::operator<(const ScopedLockGuard& other) const
{
  if (&m_lock != &other.m_lock)
  {
    return &m_lock < &other.m_lock;
  }
  // Same lock held with different lock type? Shouldn't really be possible,
  // but be defensive:
  return m_lockType < other.m_lockType;
}

ScopedLockSetGuard::ScopedLockSetGuard(
  const std::set<std::shared_ptr<Resource>>& readLockResources,
  const std::set<std::shared_ptr<Resource>>& writeLockResources)
{
  for (const auto& rsrc : readLockResources)
  {
    m_guards.emplace(rsrc->lock({}), LockType::Read);
  }
  for (const auto& rsrc : writeLockResources)
  {
    m_guards.emplace(rsrc->lock({}), LockType::Write);
  }
}

ScopedLockSetGuard::~ScopedLockSetGuard() = default;

std::unique_ptr<ScopedLockSetGuard> ScopedLockSetGuard::Block(
  const std::set<std::shared_ptr<Resource>>& readLockResources,
  const std::set<std::shared_ptr<Resource>>& writeLockResources)
{
  // This blocks until all resources are locked (or deadlocks).
  auto result = std::unique_ptr<ScopedLockSetGuard>(
    new ScopedLockSetGuard(readLockResources, writeLockResources));
  return result;
}

std::unique_ptr<ScopedLockSetGuard> ScopedLockSetGuard::Try(
  const std::set<std::shared_ptr<Resource>>& readLockResources,
  const std::set<std::shared_ptr<Resource>>& writeLockResources)
{
  // Create an empty guard.
  auto result = std::unique_ptr<ScopedLockSetGuard>(new ScopedLockSetGuard({}, {}));
  bool ok = true;

  // Start trying to acquire locks on the requested resources.
  // If any of these fails, set "ok" to false and break; we will
  // drop any previously-acquired locks before returning.
  for (const auto& rsrc : readLockResources)
  {
    auto& lock(rsrc->lock({}));
    if (lock.tryLock(LockType::Read))
    {
      result->m_guards.emplace(lock, LockType::Read, true);
    }
    else
    {
      ok = false;
      break;
    }
  }
  if (ok)
  {
    for (const auto& rsrc : writeLockResources)
    {
      auto& lock(rsrc->lock({}));
      if (lock.tryLock(LockType::Write))
      {
        result->m_guards.emplace(lock, LockType::Write, true);
      }
      else
      {
        ok = false;
        break;
      }
    }
  }
  if (!ok)
  {
    // Discard existing locks
    result = nullptr;
  }
  return result;
}

} // namespace resource
} // namespace smtk
