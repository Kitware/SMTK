//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/common/Observers.h"

#include "smtk/common/testing/cxx/helpers.h"

namespace
{
class Observed
{
public:
  typedef smtk::common::Observers<std::function<void()> > Observers;

  void operator()() { m_observers(); }

  Observers& observers() { return m_observers; }

private:
  Observers m_observers;
};
}

void TestPriority()
{
  Observed observed;

  int i = 0;

  auto first = [&]() {
    smtkTest(i == 0, "first observer was not called first");
    ++i;
  };

  auto second = [&]() {
    smtkTest(i == 1, "second observer was not called second");
    ++i;
  };

  auto third = [&]() {
    smtkTest(i == 2, "third observer was not called third");
    ++i;
  };

  observed.observers().insert(third, -3, false);
  observed.observers().insert(second, -2, false);
  observed.observers().insert(first, -1, false);

  observed();

  return;
}

int UnitTestObservers(int /*unused*/, char** const /*unused*/)
{
  TestPriority();

  return 0;
}
