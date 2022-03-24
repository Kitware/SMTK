//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/common/Visit.h"
#include "smtk/common/testing/cxx/helpers.h"

#include <initializer_list>
#include <iostream>
#include <vector>

namespace
{

class ExampleClassWithVisitMethod
{
public:
  ExampleClassWithVisitMethod(std::initializer_list<int> thingsToVisit)
    : m_thingsToVisit(thingsToVisit)
  {
  }

  // Note that this method is templated on the type of functor passed to it.
  // It "decorates" the functor passed to it with a VisitorFunctor that
  // always returns a Visit enumerant and then invokes the decorated
  // visitor instead of directly calling what the caller passed.
  // Because all the work happens at compile time, there only overhead
  // caused by the "decoration" is pushing a return value onto the
  // stack when given a \a visitor with a void return value.
  template<typename Functor>
  smtk::common::Visited visit(Functor&& visitor)
  {
    smtk::common::VisitorFunctor<Functor> decoratedVisitor(visitor);
    if (m_thingsToVisit.empty())
    {
      return smtk::common::Visited::Empty;
    }

    for (const auto& thing : m_thingsToVisit)
    {
      if (decoratedVisitor(thing) == smtk::common::Visit::Halt)
      {
        return smtk::common::Visited::Some;
      }
    }
    return smtk::common::Visited::All;
  }

protected:
  std::vector<int> m_thingsToVisit;
};

} // anonymous namespace

int UnitTestVisit(int /*unused*/, char** const /*unused*/)
{
  using smtk::common::Visit;
  using smtk::common::Visited;

  ExampleClassWithVisitMethod f{ 1, 8, 12, 2, 3 };
  ExampleClassWithVisitMethod g{};

  // Counters incremented by visitors.
  std::size_t c1 = 0;
  std::size_t c2 = 0;

  // Prepare non-simple (halting) visitors ahead of time.
  auto v2FullStop = [&c2](const int& entry) {
    ++c2;
    std::cout << "halt, @ " << entry << "\n";
    return Visit::Halt;
  };
  auto v2Exhaustive = [&c2](const int& entry) {
    ++c2;
    std::cout << "continue, @ " << entry << "\n";
    return Visit::Continue;
  };
  auto v2Terminating = [&c2](const int& entry) {
    ++c2;
    if (c2 > 7)
    {
      std::cout << "halt, @ " << entry << " (count = " << c2 << ")\n";
      return Visit::Halt;
    }
    std::cout << "continue, @ " << entry << "\n";
    return Visit::Continue;
  };

  // Test visitation of empty container.
  auto r1 = g.visit([&c1](const int& entry) {
    ++c1;
    std::cout << "simple, @ " << entry << "\n";
  });
  test(r1 == Visited::Empty, "Expected container to be empty (simple visitor).");
  test(c1 == 0, "Expected zero count with simple visitor.");

  auto r2 = g.visit(v2FullStop);
  test(r2 == Visited::Empty, "Expected container to be empty (halting visitor).");
  test(c2 == 0, "Expected zero count with halting visitor.");

  // Simple visitor, non-empty container
  r1 = f.visit([&c1](const int& entry) {
    ++c1;
    std::cout << "simple, @ " << entry << "\n";
  });
  test(r1 == Visited::All, "Expected to visit all entries with simple visitor.");
  test(c1 == 5, "Expected to count all entries with simple visitor.");

  // Halting visitor, non-empty container, exhaustive iteration
  r2 = f.visit(v2Exhaustive);
  test(r2 == Visited::All, "Expected to visit all entries with halting visitor.");
  test(c2 == 5, "Expected to count all entries with halting visitor.");

  // Halting visitor, non-empty container, early termination
  r2 = f.visit(v2Terminating);
  test(r2 == Visited::Some, "Expected to terminate early this time.");
  test(c2 == 8, "Expected a partial count this time.");

  return 0;
}
