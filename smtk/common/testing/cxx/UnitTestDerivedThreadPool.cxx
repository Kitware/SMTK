//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/common/ThreadPool.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <chrono>
#include <iostream>

namespace
{
template <typename ReturnType>
class StoppableThreadPool : public smtk::common::ThreadPool<ReturnType>
{
public:
  StoppableThreadPool(unsigned int maxThreads = 0)
    : smtk::common::ThreadPool<ReturnType>(maxThreads)
  {
  }

  void stop() { m_stopped = true; }
  void start() { m_stopped = false; }

private:
  void exec() override
  {
    while (this->m_active)
    {
      std::packaged_task<ReturnType()> task;
      {
        std::unique_lock<std::mutex> queueLock(this->m_queueMutex);

        // Access a task from the queue.
        this->m_condition.wait(queueLock, [this] { return !this->m_queue.empty(); });
        if (!m_stopped)
        {
          task = std::move(this->m_queue.front());
        }
        else
        {
          std::packaged_task<ReturnType()> emptyTask([]() { return ReturnType(); });
          task = std::move(emptyTask);
        }

        this->m_queue.pop();
      }

      // Execute the task.
      task();
    }
  }

  bool m_stopped{ false };
};
}

int UnitTestDerivedThreadPool(int, char** const)
{
  StoppableThreadPool<bool> threadPool(2);
  std::vector<std::future<bool> > futures;
  int n_threads = 2;
  std::this_thread::sleep_for(std::chrono::seconds(1));
  for (int i = 0; i < n_threads; ++i)
  {
    std::future<bool> future = threadPool([]() { return true; });
    futures.emplace_back(std::move(future));
  }

  threadPool.stop();

  for (auto& future : futures)
  {
    future.wait();
  }

  return 0;
}
