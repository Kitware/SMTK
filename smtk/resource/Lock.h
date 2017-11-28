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
#include <mutex>

namespace smtk
{
namespace resource
{

// enum class does not need to be exported
// Reference link: https://groups.google.com/a/chromium.org/forum/#!topic/chromium-dev/dfSl9Ypdq6Y
enum class Permission
{
  Read,
  Write
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

  SMTKCORE_EXPORT void lock(Permission);
  SMTKCORE_EXPORT void unlock(Permission);

private:
  std::mutex m_mutex;
  std::condition_variable m_readerCondition;
  std::condition_variable m_writerCondition;
  std::size_t m_activeReaders;
  std::size_t m_waitingWriters;
  std::size_t m_activeWriters;
};
}
}

#endif // smtk_resource_Lock_h
