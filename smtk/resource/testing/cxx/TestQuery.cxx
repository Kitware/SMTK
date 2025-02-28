//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/resource/query/DerivedFrom.h"
#include "smtk/resource/query/Factory.h"
#include "smtk/resource/query/Query.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <cmath>
#include <tuple>

namespace
{
constexpr double EPSILON = 1.e-8;

struct QueryA : smtk::resource::query::DerivedFrom<QueryA, smtk::resource::query::Query>
{
  ~QueryA() override = default;
  [[nodiscard]] virtual double foo() const { return 2.; }
};

struct QueryB : smtk::resource::query::DerivedFrom<QueryB, QueryA>
{
  [[nodiscard]] double foo() const override { return 3.; }
  [[nodiscard]] int bar() const { return 3; }
};

struct QueryC : smtk::resource::query::DerivedFrom<QueryC, QueryA>
{
  [[nodiscard]] double foo() const override { return 4.; }
};

struct QueryD : smtk::resource::query::DerivedFrom<QueryD, QueryC>
{
  [[nodiscard]] double foo() const override { return 6.; }
};
} // namespace

int TestQuery(int /*unused*/, char** const /*unused*/)
{
  smtk::resource::query::Factory factory;

  test(factory.registerQuery<QueryA>(), "Could not register QueryA");
  {
    std::unique_ptr<QueryA> queryA = factory.create<QueryA>();
    test(std::abs(queryA->foo() - QueryA().foo()) < EPSILON, "Unexpected query result");
  }
  test(factory.unregisterQuery<QueryA>(), "Could not unregister QueryA");

  test(
    factory.registerQuery<QueryB>([](const std::size_t& index) {
      if (index == QueryB::typeIndex())
        return std::numeric_limits<int>::max();
      return -1;
    }),
    "Could not register QueryB with a custom priority function");
  {
    std::unique_ptr<QueryB> queryB = factory.create<QueryB>();
    test(queryB != nullptr, "Could not create an instance of QueryB");
    test(std::abs(queryB->foo() - QueryB().foo()) < EPSILON, "Unexpected query result");
    test(std::abs(queryB->bar() - QueryB().bar()) < EPSILON, "Unexpected query result");
  }

  test(factory.registerQuery<QueryA>(), "Could not register QueryA");
  {
    std::unique_ptr<QueryA> queryA = factory.create<QueryA>();
    test(std::abs(queryA->foo() - QueryA().foo()) < EPSILON, "Unexpected query result");
  }

  test(factory.registerQuery<QueryC>(), "Could not register QueryC");
  {
    std::unique_ptr<QueryA> queryC = factory.create<QueryA>();
    test(std::abs(queryC->foo() - QueryC().foo()) < EPSILON, "Unexpected query result");
  }

  test(factory.registerQuery<QueryD>(), "Could not register QueryD");
  {
    std::unique_ptr<QueryA> queryD = factory.create<QueryA>();
    test(std::abs(queryD->foo() - QueryD().foo()) < EPSILON, "Unexpected query result");
  }

  return 0;
}
