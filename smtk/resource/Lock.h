//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_resource_Lock_h
#define smtk_resource_Lock_h

#include "smtk/CoreExports.h"

#include <condition_variable>
#include <memory>
#include <mutex>
#include <set>

namespace smtk
{
namespace resource
{

class Resource;

// enum class does not need to be exported
// Reference link: https://groups.google.com/a/chromium.org/forum/#!topic/chromium-dev/dfSl9Ypdq6Y
enum class LockType
{
  DoNotLock = 0,
  Unlocked = 0,
  Read,
  Write,
};

/// A read/write lock for resources. This lock is designed to potentially starve
/// readers in favor of writers. This is not necessarily bad; it means that
/// writers are given priority over readers when there are multiple readers and
/// writers simultaneously attempting to access the resource.
class Lock
{
public:
  SMTKCORE_EXPORT Lock();
  Lock(const Lock&) = delete;
  Lock& operator=(const Lock&) = delete;

  SMTKCORE_EXPORT void lock(LockType);
  SMTKCORE_EXPORT bool tryLock(LockType);
  SMTKCORE_EXPORT void unlock(LockType);

  SMTKCORE_EXPORT LockType state() const;

private:
  std::mutex m_mutex;
  std::condition_variable m_readerCondition;
  std::condition_variable m_writerCondition;
  std::size_t m_activeReaders{ 0 };
  std::size_t m_waitingWriters{ 0 };
  std::size_t m_activeWriters{ 0 };
};

/// A scope-guarded utility for handling locks.
///
/// As long as an instance of this class is alive, the lock passed to
/// its constructor is held. Upon destruction of this instance, the
/// lock is released.
class SMTKCORE_EXPORT ScopedLockGuard
{
public:
  /// A constructor that blocks until it is able to acquire a \a lock of the given \a lockType.
  ScopedLockGuard(Lock&, LockType);

  /// Construct a guard without acquiring the lock (because it is already held by the caller).
  ///
  /// A constructor for ScopedLockSetGuard to use when tryLock() has already
  /// incremented lock counters for us. The last argument is ignored; it is
  /// used to differentiate between the public constructor and this one.
  ScopedLockGuard(Lock& alreadyAcquiredLock, LockType lockType, bool alreadyAcquired);

  /// Release the lock held at construction time in this destructor.
  ~ScopedLockGuard();

  /// A comparator so guards can be held in ordered containers.
  bool operator<(const ScopedLockGuard& other) const;

private:
  Lock& m_lock;
  LockType m_lockType;
};

/**\brief A utility for holding multiple lock-guards at once.
  *
  * This object's static Block() and Try() methods take a set
  * of resources to read-lock and a set of resources to write-lock. The
  * constructor will block until all resources are locked before returning
  * while ScopedLockSetGuard::Try() always return immediately but
  * may return a null pointer (when resource locks are not immediately
  * available).
  *
  * Note that the public API always returns unique pointers to these
  * objects; you may convert the unique pointer to a shared pointer
  * if you wish (so that resources are held until all shared owners
  * complete), but you may not copy-construct or assign the guard.
  */
class ScopedLockSetGuard
{
public:
  SMTKCORE_EXPORT ~ScopedLockSetGuard();

  /// Do not allow copy-construction or assignment of lock guards.
  ScopedLockSetGuard(const ScopedLockSetGuard&) = delete;
  void operator=(const ScopedLockSetGuard&) = delete;

  SMTKCORE_EXPORT static std::unique_ptr<ScopedLockSetGuard> Block(
    const std::set<std::shared_ptr<Resource>>& readLockResources,
    const std::set<std::shared_ptr<Resource>>& writeLockResources);

  SMTKCORE_EXPORT static std::unique_ptr<ScopedLockSetGuard> Try(
    const std::set<std::shared_ptr<Resource>>& readLockResources,
    const std::set<std::shared_ptr<Resource>>& writeLockResources);

protected:
  ScopedLockSetGuard(
    const std::set<std::shared_ptr<Resource>>& readLockResources,
    const std::set<std::shared_ptr<Resource>>& writeLockResources);

  std::set<ScopedLockGuard> m_guards;
};

} // namespace resource
} // namespace smtk

#endif // smtk_resource_Lock_h
