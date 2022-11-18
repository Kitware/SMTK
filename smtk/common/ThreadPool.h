//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_common_ThreadPool_h
#define smtk_common_ThreadPool_h

#include "smtk/common/CompilerInformation.h"
#include "smtk/io/Logger.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <vector>

namespace smtk
{
namespace common
{
/// A basic thread pool that executes functors in separate threads. It accepts
/// the number of threads to build at construction, and waits for all tasks to
/// complete before being destroyed. The \a ReturnType is a default-constructible
/// type that tasks return via std::future.
template<typename ReturnType = void>
class SMTK_ALWAYS_EXPORT ThreadPool
{
  static_assert(
    std::is_same<ReturnType, void>::value || std::is_default_constructible<ReturnType>::value,
    "Templated return type must be void or a default constructible type");

public:
  /// Initialize thread pool with \a maxThreads threads spawned to execute tasks.
  ThreadPool(unsigned int maxThreads = 0);
  virtual ~ThreadPool();

  /// Add a task to be performed by the thread queue. Once a thread becomes
  /// available, it will pop the task from the queue and execute it. The return
  /// value can be accessed from the returned future.
  template<typename Function, typename... Types>
  std::future<ReturnType> operator()(Function&& function, Types&&... args)
  {
    return appendToQueue(std::bind(function, std::forward<Types>(args)...));
  }

protected:
  void initialize();

  /// Append a functor with no inputs to the task queue. This is used in tandem
  /// with std::bind to construct the class's call method.
  std::future<ReturnType> appendToQueue(std::function<ReturnType()>&& task);

  /// Run by a worker thread: poll the task queue for tasks to perform.
  /// NOTE: the single layer of misdirection ensures that we launch a thread
  ///       using the correct method address.
  void execute() { return this->exec(); }
  virtual void exec();

  std::condition_variable m_condition;
  std::mutex m_queueMutex;
  std::vector<std::thread> m_threads;
  std::queue<std::packaged_task<ReturnType()>> m_queue;
  bool m_initialized{ false };
  std::atomic<bool> m_active;
  unsigned int m_maxThreads;
};

template<typename ReturnType>
ThreadPool<ReturnType>::ThreadPool(unsigned int maxThreads)
  : m_active(true)
  , m_maxThreads(maxThreads == 0 ? std::thread::hardware_concurrency() : maxThreads)
{
}

template<typename ReturnType>
ThreadPool<ReturnType>::~ThreadPool()
{
  // Change the state of the thread pool to signify that threads should no
  // longer run.
  m_active = false;

  // For each thread in the pool, send a task that will cause it to exit its
  // infinite loop.
  for (std::size_t i = 0; i < m_threads.size(); i++)
  {
    (*this)([] { return ReturnType(); });
  }

  // Now that all of the threads have exited, join each thread with the parent
  // thread.
  for (auto& thread : m_threads)
  {
    thread.join();
  }
}

template<typename ReturnType>
void ThreadPool<ReturnType>::initialize()
{
  for (unsigned int i = 0; i < m_maxThreads; ++i)
  {
    m_threads.push_back(std::thread(&ThreadPool<ReturnType>::execute, this));
  }
}

template<typename ReturnType>
std::future<ReturnType> ThreadPool<ReturnType>::appendToQueue(std::function<ReturnType()>&& task)
{
  std::future<ReturnType> future;

  // Scope access to the queue.
  {
    std::unique_lock<std::mutex> queueLock(m_queueMutex);

    // If the thread pool is not yet active, initialize it.
    // NOTE: we use lazy initialization here to allow derived thread pools to
    //       have custom exec() logic, which is called upon the construction of
    //       std::thread. The derived methods will only be available after
    //       the base constructor has returned, so we cannot construct
    //       std::thread instances in this class's constructor.
    if (!m_initialized)
    {
      m_initialized = true;
      this->initialize();
    }

    // Construct a packaged_task to launch the input task and access its
    // future.
    m_queue.emplace(task);
    future = m_queue.back().get_future();
  }
  // Signal to the next thread that the queue is ready for access.
  m_condition.notify_one();

  // Return the future associated with the promise created above.
  return future;
}

template<typename ReturnType>
void ThreadPool<ReturnType>::exec()
{
  // Always check if the containing class has signaled that the thread pool
  // should no longer be active.
  while (m_active)
  {
    std::packaged_task<ReturnType()> task;
    // Scope access to the queue.
    {
      std::unique_lock<std::mutex> queueLock(m_queueMutex);

      // Access a task from the queue.
      m_condition.wait(queueLock, [this] { return !m_queue.empty(); });
      task = std::move(m_queue.front());
      m_queue.pop();
    }
    try
    {
      // Execute the task.
      task();
    }
    catch (...)
    {
      // At least let the user know something went wrong.
      smtkErrorMacro(smtk::io::Logger::instance(), "Uncaught thread task exception.");
    }
  }
}
} // namespace common
} // namespace smtk

#endif // smtk_common_ThreadPool_h
