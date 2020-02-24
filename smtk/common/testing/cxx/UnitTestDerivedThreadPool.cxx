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
#include <cstring>

// ubsan is a fickle beast. This define circumvents the error "runtime error:
// member access within address <> which does not point to an object of type
// 'StoppableThreadPool'" when m_stopped is referenced in StoppableThreadPool's
// exec() method, which we think is a false positive.
#if __has_attribute(no_sanitize)
#define NO_UBSAN_VPTR __attribute__((no_sanitize("vptr")))
#else
#define NO_UBSAN_VPTR
#endif

namespace
{
class StoppableThreadPool : public smtk::common::ThreadPool<bool>
{
public:
  StoppableThreadPool(unsigned int maxThreads = 0)
    : smtk::common::ThreadPool<bool>(maxThreads)
  {
  }

  void stop() { m_stopped = true; }
  void start() { m_stopped = false; }

private:
  NO_UBSAN_VPTR void exec() override
  {
    while (this->m_active)
    {
      std::packaged_task<bool()> task;
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
          std::packaged_task<bool()> emptyTask([]() { return bool(); });
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

int UnitTestDerivedThreadPool(int /*unused*/, char** const /*unused*/)
{
  StoppableThreadPool threadPool(2);
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
