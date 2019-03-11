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
void task3(int i)
{
  std::cout << "Hello from task 3, worker thread " << i << std::endl;
}
}

int UnitTestThreadPool(int, char** const)
{
  {
    smtk::common::ThreadPool<void>* pool = new smtk::common::ThreadPool<void>();

    (*pool)([] { std::cout << "Hello from one of the worker threads" << std::endl; });

    auto task2 = [](int i) { std::cout << "Hello from worker thread " << i << std::endl; };
    (*pool)(std::bind(task2, 3));

    (*pool)(task3, 5);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    (*pool)([] {
      std::cout << "forcing worker to outlive pool" << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });
    delete pool;
  }

  {
    smtk::common::ThreadPool<int>* pool = new smtk::common::ThreadPool<int>();

    std::future<int> result = (*pool)([] { return 1; });

    smtkTest(result.get() == 1, "Returned result doesn't match input");

    auto task2 = [](int i) {
      std::cout << "Hello from worker thread " << i << std::endl;
      return i;
    };
    (*pool)(std::bind(task2, 3));

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    (*pool)([] {
      std::cout << "forcing worker to outlive pool" << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      return 2;
    });
    delete pool;
  }

  return 0;
}
