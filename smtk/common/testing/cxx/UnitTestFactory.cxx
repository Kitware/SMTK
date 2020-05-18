//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/common/Factory.h"

#include "nlohmann/json.hpp"

#include "smtk/common/testing/cxx/helpers.h"

namespace test_space
{
namespace basic
{
struct Base
{
  Base() = default;
  Base(const std::string& val)
    : value(val)
  {
  }
  virtual ~Base() = default;
  std::string value;
};

class Derived : public Base
{
public:
  Derived() { value += "_derived"; }
  Derived(const std::string& val)
    : Base(val)
  {
    value += "_derived";
  }
};
} // namespace basic

namespace json
{
struct Base
{
  Base() = default;
  Base(const std::string& val)
    : value(val)
  {
  }
  Base(const nlohmann::json& json);
  virtual ~Base() = default;
  std::string value;
};

class Derived : public Base
{
public:
  Derived() { value += "_derived"; }
  Derived(const std::string& val)
    : Base(val)
  {
    value += "_derived";
  }
  Derived(const nlohmann::json& json);
};

void to_json(nlohmann::json& j, const Base& base)
{
  j["value"] = base.value;
}

void from_json(const nlohmann::json& j, Base& base)
{
  base.value = j.at("value").get<std::string>();
}

void to_json(nlohmann::json& j, const Derived& derived)
{
  j["value"] = derived.value;
}

void from_json(const nlohmann::json& j, Derived& derived)
{
  derived.value = j.at("value").get<std::string>();
}

Base::Base(const nlohmann::json& json)
{
  (*this) = json.get<Base>();
}

Derived::Derived(const nlohmann::json& json)
{
  (*this) = json.get<Derived>();
}
} // namespace json

namespace two_parameters
{
struct Base
{
  Base() = delete;
  Base(const std::string& val1, const int& val2)
    : value1(val1)
    , value2(val2)
  {
  }
  virtual ~Base() = default;
  std::string value1;
  int value2;
};

class Derived : public Base
{
public:
  Derived(const std::string& val1, const int& val2)
    : Base(val1, val2)
  {
    value1 += "_derived";
    value2 += 1;
  }
};
} // namespace two_parameters
} // namespace test_space

int UnitTestFactory(int /*unused*/, char** const /*unused*/)
{
  using namespace smtk::common::factory;

  {
    smtk::common::Factory<test_space::basic::Base, void, std::string> factory;

    test(
      !factory.contains<test_space::basic::Derived>(),
      "Factory instance should not have type registered to it");

    factory.registerType<test_space::basic::Derived>();

    test(
      factory.contains<test_space::basic::Derived>(),
      "Factory instance should have type registered to it");

    {
      auto my_derived = factory.create<test_space::basic::Derived>();
      test(my_derived && (my_derived->value == "_derived"), "Type not created properly");
    }

    {
      auto my_derived = factory.createFromName("test_space::basic::Derived");
      test(my_derived && (my_derived->value == "_derived"), "Type not created properly");
    }

    {
      auto my_derived = factory.get<Name>().create("test_space::basic::Derived");
      test(my_derived && (my_derived->value == "_derived"), "Type not created properly");
    }

    {
      auto my_derived = factory.get<Index>().create(typeid(test_space::basic::Derived).hash_code());
      test(my_derived && (my_derived->value == "_derived"), "Type not created properly");
    }

    auto my_derived_wstring =
      factory.createFromName("test_space::basic::Derived", std::string("bar"));

    test(
      my_derived_wstring && (my_derived_wstring->value == "bar_derived"),
      "Type not created properly");

    factory.unregisterType<test_space::basic::Derived>();

    test(
      !factory.contains<test_space::basic::Derived>(),
      "Factory instance should not have type registered to it");
  }

  {
    smtk::common::Factory<test_space::json::Base, std::string, nlohmann::json> factory;

    test(
      !factory.contains<test_space::json::Derived>(),
      "Factory instance should not have type registered to it");

    factory.registerType<test_space::json::Derived>();

    test(
      factory.contains<test_space::json::Derived>(),
      "Factory instance should have type registered to it");

    auto my_derived = factory.create<test_space::json::Derived>(std::string("bar"));

    test(my_derived && (my_derived->value == "bar_derived"), "Type not created properly");

    nlohmann::json json = *my_derived;

    {
      auto my_derived2 = factory.createFromName("test_space::json::Derived", json);
      test(my_derived2 && (my_derived2->value == "bar_derived"), "Type not created properly");
    }

    {
      auto my_derived2 = factory.get<Name>().create("test_space::json::Derived", json);
      test(my_derived2 && (my_derived2->value == "bar_derived"), "Type not created properly");
    }

    factory.unregisterType<test_space::json::Derived>();

    test(
      !factory.contains<test_space::json::Derived>(),
      "Factory instance should not have type registered to it");
  }

  {
    smtk::common::Factory<test_space::two_parameters::Base, Inputs<std::string, int>> factory;

    test(
      !factory.contains<test_space::two_parameters::Derived>(),
      "Factory instance should not have type registered to it");

    typedef std::tuple<test_space::two_parameters::Derived> Types;
    factory.registerTypes<Types>();

    test(
      factory.contains<test_space::two_parameters::Derived>(),
      "Factory instance should have type registered to it");

    {
      auto my_derived = factory.create<test_space::two_parameters::Derived>(std::string("bar"), 1);
      test(
        my_derived && (my_derived->value1 == "bar_derived") && (my_derived->value2 == 2),
        "Type not created properly");
    }

    auto my_derived_wstring =
      factory.createFromName("test_space::two_parameters::Derived", std::string("bar"), 1);

    test(
      my_derived_wstring && (my_derived_wstring->value1 == "bar_derived") &&
        (my_derived_wstring->value2 == 2),
      "Type not created properly");

    factory.unregisterTypes<Types>();

    test(
      !factory.contains<test_space::two_parameters::Derived>(),
      "Factory instance should not have type registered to it");
  }

  return 0;
}
