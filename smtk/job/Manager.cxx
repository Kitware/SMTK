//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/job/Manager.h"

namespace smtk
{
namespace job
{

Manager::Manager()
  : m_activeQueue(&m_queues)
{
}

const QueueInstances& Manager::queues() const
{
  return m_queues;
}

QueueInstances& Manager::queues()
{
  return m_queues;
}

smtk::common::Active<QueueInstances>& Manager::activeQueue()
{
  return m_activeQueue;
}

Queue* Manager::defaultQueue() const
{
  return m_activeQueue.object();
}

} // namespace job
} // namespace smtk
