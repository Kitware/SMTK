//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME Logger.h -
// .SECTION Description
// .SECTION See Also

#include "smtk/io/Logger.h"
#include <chrono>
#include <iostream>
#include <thread>

void foo(int i)
{
  smtkErrorMacro(smtk::io::Logger::instance(), "Hey I'm running in a thread! - i = " << i);
}
int main()
{
  std::vector<std::thread> threads;
  threads.reserve(10);
  for (int i = 0; i < 10; i++)
  {
    threads.emplace_back(foo, i);
  }

  for (auto& th : threads)
  {
    th.join();
  }
  return 0;
}
