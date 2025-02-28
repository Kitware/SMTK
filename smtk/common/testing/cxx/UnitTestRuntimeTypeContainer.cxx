//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/common/CompilerInformation.h" // for expectedKeys
#include "smtk/common/RuntimeTypeContainer.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <cmath>

namespace
{
const double float_epsilon = 1.e-6;

struct Foo
{
  Foo(int i)
    : value(i)
  {
  }
  int value;
};

struct Bar
{
  Bar(int i)
    : value(i)
  {
  }
  Bar(const Bar& other) = default;
  Bar(Bar&& other) = default;

  int value;
};

struct Base
{
  Base() = default;
  Base(const std::string& suffix)
    : m_data("_" + suffix)
  {
  }
  Base(const Base&) = default;
  Base(Base&&) = default;
  Base& operator=(const Base&) = default;
  bool operator==(const Base& other) { return m_data == other.m_data; }
  std::string m_data;
  [[nodiscard]] virtual std::string name() const { return "Base" + m_data; }
};

struct Derived : public Base
{
  Derived() = default;
  Derived(const std::string& suffix)
    : Base("d1_" + suffix)
  {
  }
  Derived(const Derived&) = default;
  Derived(Derived&&) = default;
  Derived& operator=(const Derived&) = default;
  [[nodiscard]] std::string name() const override { return "Derived" + m_data; }
};

struct Derived2 : public Derived
{
  Derived2() = default;
  Derived2(const std::string& suffix)
    : Derived("d2_" + suffix)
  {
  }
  Derived2(const Derived2&) = default;
  Derived2(Derived2&&) = default;
  Derived2& operator=(const Derived2&) = default;
  [[nodiscard]] std::string name() const override { return "Derived2" + m_data; }
};

} // namespace

int UnitTestRuntimeTypeContainer(int /*unused*/, char** const /*unused*/)
{
  using namespace smtk::string::literals;
  smtk::common::RuntimeTypeContainer typeContainer;

  // I. First, test that all the base-class TypeContainer stuff works as usual:
  test(typeContainer.empty(), "New instance shoud contain no values.");
  test(!typeContainer.contains<int>(), "New instance should contain no values of integer type.");

  typeContainer.get<int>() = 3;

  test(typeContainer.size() == 1, "Assigned value should increment the container size.");
  test(typeContainer.get<int>() == 3, "Assigned value should be retrievable.");

  typeContainer.clear();

  test(typeContainer.empty(), "Cleared instance shoud contain no values.");
  test(
    !typeContainer.contains<int>(), "Cleared instance should contain no values of integer type.");

  typeContainer.insert<float>(2.3f);

  test(
    fabs(typeContainer.get<float>() - 2.3f) < float_epsilon,
    "Assigned value should be retrievable.");

  try
  {
    typeContainer.get<Foo>() = Foo(3);
    test(false, "Access to a type with no default constructor should throw an error.");
  }
  catch (const std::out_of_range&)
  {
  }

  typeContainer.emplace<Foo>(3);
  test(typeContainer.get<Foo>().value == 3, "Assigned value should be retrievable.");

  typeContainer.insert<Bar>(Bar(2));
  test(typeContainer.get<Bar>().value == 2, "Assigned value should be retrievable.");

  // II. Now test RuntimeTypeContainer-specific methods.
  typeContainer.insertRuntime<Base>("NotReallyBase", Derived("nr"));
  typeContainer.insertRuntime<Base>("FarFromBase", Derived2("ff"));
  typeContainer.emplaceRuntime<Base>("(anonymous namespace)::Base", "bb");
  auto base0Name = typeContainer.getRuntime<Base>("(anonymous namespace)::Base").name();
  auto base1Name = typeContainer.getRuntime<Base>("NotReallyBase").name();
  auto base2Name = typeContainer.getRuntime<Base>("FarFromBase").name();
  // clang-format off
  std::cout
    << "Runtime insertion via base class produced:\n"
    << " 1. " << base0Name << "\n"
    << " 2. " << base1Name << "\n"
    << " 3. " << base2Name << "\n"
    ;
  // clang-format on
  test(base0Name == "Base_bb", "Did not properly emplace base with non-default ctor.");
  test(base1Name == "Derived_d1_nr", "Did not properly insert derived with non-default ctor.");
  test(base2Name == "Derived2_d1_d2_ff", "Did not properly insert derived2 with non-default ctor.");

  // Test that we can fetch "special" runtime objects using the inherited API.
  // (Special in the sense that their declared type-name is their actual type-name.)
  base2Name = typeContainer.get<Base>().name();
  std::cout << "Fetched via base get<>() API: " << base2Name << "\n";
  test(base2Name == "Base_bb", "Failed to fetch specially-named base object with inherited API.");

  // Test that insertOrAssignRuntime() works
  Derived2 dummy;
  typeContainer.insertOrAssignRuntime<Base>("(anonymous namespace)::Base", dummy);
  base1Name = typeContainer.get<Base>().name();
  std::cout << "Assigning to existing key (should overwrite) produced " << base1Name << "\n";
  test(base1Name == "Derived2", "Failed to overwrite existing storage with new value.");

  dummy = Derived2("dd");
  typeContainer.insertOrAssignRuntime<Base>("NewlyInserted", dummy);
  test(typeContainer.containsRuntime("NewlyInserted"), "Did not insert new value.");
  base1Name = typeContainer.getRuntime<Base>("NewlyInserted").name();
  std::cout << "Assigning to new key (should insert) produced " << base1Name << "\n";
  test(base1Name == "Derived2_d1_d2_dd", "Failed to overwrite existing storage with new value.");

  // III. Test copying of RuntimeTypeContainer objects.
  smtk::common::RuntimeTypeContainer typeContainer2(typeContainer);
  test(typeContainer2.get<Foo>().value == 3, "Copied container should behave like the original.");
  test(typeContainer2.get<Bar>().value == 2, "Copied container should behave like the original.");
  for (const auto& runtimeTypeName :
       typeContainer2.runtimeTypeNames(smtk::common::typeName<Base>()))
  {
    std::ostringstream msg;
    msg << "Expected copied type container to have matching " << runtimeTypeName.data() << ".";
    test(
      typeContainer.getRuntime<Base>(runtimeTypeName) ==
        typeContainer2.getRuntime<Base>(runtimeTypeName),
      msg.str());
  }

  smtk::common::RuntimeTypeContainer typeContainer3(
    typeContainer2.get<Foo>(), typeContainer2.get<Bar>());
  test(
    typeContainer3.get<Foo>().value == 3,
    "Variadic constructed container should behave like the original.");
  test(
    typeContainer3.get<Bar>().value == 2,
    "Variadic constructed container should behave like the original.");

  // IV. Print out membership of out copied type container:
  std::set<smtk::string::Token> expectedKeys{ { "float"_token,
                                                "(anonymous namespace)::Foo"_token,
                                                "(anonymous namespace)::Bar"_token,
                                                "(anonymous namespace)::Base",
                                                "NotReallyBase",
                                                "FarFromBase",
                                                "NewlyInserted" } };
  std::unordered_set<smtk::string::Token> expectedRuntimeTypeNames{
    { "(anonymous namespace)::Base", "NotReallyBase", "FarFromBase", "NewlyInserted" }
  };
  smtk::string::Token baseTypeName;
  {
    std::cout << "Type container now holds:\n";
    for (const auto& token : typeContainer2.keys())
    {
      std::cout << "  " << token.data() << " (" << std::hex << token.id() << std::dec << ")\n";
    }
    test(typeContainer2.keys() == expectedKeys, "Container keys were improperly reported.");

    std::cout << "Runtime base-class types:\n";
    for (const auto& token : typeContainer2.runtimeBaseTypes())
    {
      baseTypeName = token;
      std::cout << "  " << token.data() << "\n";
      for (const auto& token2 : typeContainer2.runtimeTypeNames(token))
      {
        std::cout << "    " << token2.data() << "\n";
      }
    }

    test(
      typeContainer2.runtimeTypeNames(baseTypeName) == expectedRuntimeTypeNames,
      "Run-time type information was not kept up-to-date.");
  }

  test(typeContainer2.erase<Base>(), "Did not erase eponymous Base object.");
  expectedRuntimeTypeNames.erase("(anonymous namespace)::Base");
  test(
    typeContainer2.runtimeTypeNames(baseTypeName) == expectedRuntimeTypeNames,
    "Run-time type information was not kept up-to-date during templated erasure.");

  test(typeContainer2.eraseRuntime("NewlyInserted"), "Did not erase NewlyInserted object.");
  expectedRuntimeTypeNames.erase("NewlyInserted");
  test(
    typeContainer2.runtimeTypeNames(baseTypeName) == expectedRuntimeTypeNames,
    "Run-time type information was not kept up-to-date during runtime erasure.");

  // Ensure traditional erasure with no runtime information still succeeds.
  test(typeContainer2.erase<float>(), "Did not erase float object.");

  test(typeContainer2.size() == expectedKeys.size() - 3, "Failed to erase 2 keys.");
  return 0;
}
