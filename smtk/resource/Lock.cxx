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

namespace smtk
{
namespace resource
{

Lock::Lock()
  : m_mutex()
  , m_activeReaders(0)
  , m_waitingWriters(0)
  , m_activeWriters(0)
{
}

void Lock::lock(Permission permission)
{
  if (permission == Permission::Read)
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
  else if (permission == Permission::Write)
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

void Lock::unlock(Permission permission)
{
  if (permission == Permission::Read)
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
  else if (permission == Permission::Write)
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

} // namespace resource
} // namespace smtk
