//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/common/Singletons.h"

#include "smtk/common/TypeContainer.h"

#include <mutex>

namespace smtk
{
namespace common
{
namespace
{

// rely on default initialization to 0 for static variables
unsigned int s_singletonsCleanupCounter;
TypeContainer* s_singletons;
std::mutex* s_singletonsMutex;

} // anonymous namespace

namespace detail
{

singletonsCleanup::singletonsCleanup()
{
  if (++s_singletonsCleanupCounter == 1)
  {
    s_singletons = nullptr;
    s_singletonsMutex = new std::mutex;
  }
}

singletonsCleanup::~singletonsCleanup()
{
  if (--s_singletonsCleanupCounter == 0)
  {
    finalizeSingletons();
    delete s_singletonsMutex;
  }
}

} // namespace detail

TypeContainer& singletons()
{
  std::lock_guard<std::mutex> guard(*s_singletonsMutex);
  if (!s_singletons)
  {
    s_singletons = new TypeContainer;
  }
  return *s_singletons;
}

void finalizeSingletons()
{
  // Per @michael.migliore this can cause exceptions on macos in cases
  // when the mutex is destroyed before finalizeSingletons() is called:
  // std::lock_guard<std::mutex> guard(s_singletonsMutex);

  delete s_singletons;
  s_singletons = nullptr;
}

} // namespace common
} // namespace smtk
