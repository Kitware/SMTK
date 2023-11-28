//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/common/TypeHierarchy.h"

#include "smtk/model/Entity.h"
#include "smtk/model/Resource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/Resource.h"
#include "smtk/common/testing/cxx/helpers.h"

#include "smtk/SharedFromThis.h"

#include <iostream>
#include <memory>

#include <unordered_map>
#include <vector>

namespace smtk
{
namespace test
{
class Foo
{
public:
  smtkTypeMacroBase(Foo);
};

class Bar : public Foo
{
public:
  smtkTypeMacro(Bar);
  smtkSuperclassMacro(Foo);
};

class Baz : public Bar
{
public:
  smtkTypeMacro(Baz);
  smtkSuperclassMacro(Bar);
};

class Xyzzy : public Bar
{
public:
  smtkTypeMacro(Xyzzy);
  smtkSuperclassMacro(Bar);
};
} // namespace test
} // namespace smtk

namespace
{

template<typename Type, typename Stop = void>
bool validateHierarchy(const std::vector<std::string>& expected)
{
  bool ok = true;
  std::vector<smtk::string::Token> linear;
  std::unordered_set<smtk::string::Token> set;

  smtk::common::typeHierarchy<Type, Stop>(linear);
  smtk::common::typeHierarchy<Type, Stop>(set);
  std::size_t n = 0;
  if (std::is_same<Stop, void>::value)
  {
    std::cout << "Testing typeHierarchy<" << smtk::common::typeName<Type>() << ">:\n";
  }
  else
  {
    std::cout << "Testing typeHierarchy<" << smtk::common::typeName<Type>() << ", "
              << smtk::common::typeName<Stop>() << ">:\n";
  }
  // Print out the linear type-hierarchy:
  for (const auto& entry : linear)
  {
    std::cout << std::string(2 * (n + 1), ' ') << entry.data() << "\n";
    ++n;
  }

  n = 0;
  for (const auto& entry : linear)
  {
    if (entry.data() != expected[n])
    {
      std::cerr << n << ": Expected " << expected[n] << " got " << entry.data() << "\n";
      ok = false;
    }
    if (set.find(expected[n]) == set.end())
    {
      std::cerr << "Set did not contain \"" << expected[n] << "\".\n";
      ok = false;
    }
    ++n;
  }
  ok &= (expected.size() == linear.size());
  std::cout << "\n";
  return ok;
}

bool validateGenerationsFrom(
  const std::shared_ptr<smtk::resource::PersistentObject>& obj,
  const std::vector<std::pair<smtk::string::Token, std::size_t>>& expected)
{
  if (!obj)
  {
    return false;
  }

  bool ok = true;
  std::cout << "Generations of " << obj->typeName() << " from:\n";
  for (const auto& entry : expected)
  {
    auto dist = obj->generationsFromBase(entry.first);
    std::cout << "  " << entry.first.data() << ": " << dist << " (expect " << entry.second << ")\n";
    ok &= (dist == entry.second);
  }
  std::cout << "\n";
  return ok;
}

} // anonymous namespace

int UnitTestTypeHierarchy(int /*unused*/, char** const /*unused*/)
{
  using namespace std;

  // Test that inheritance trees are properly computed:
  test(
    validateHierarchy<smtk::test::Xyzzy>({ "Xyzzy", "Bar", "Foo" }),
    "Failed full anonymous hierarchy.");

  test(
    validateHierarchy<smtk::test::Baz, smtk::test::Foo>({ "Baz", "Bar" }),
    "Failed partial anonymous hierarchy.");

  test(
    validateHierarchy<smtk::model::Resource>({ "smtk::model::Resource",
                                               "smtk::geometry::Resource",
                                               "smtk::resource::Resource",
                                               "smtk::resource::PersistentObject" }),
    "Failed resource hierarchy");

  test(
    validateHierarchy<smtk::model::Entity>(
      { "smtk::model::Entity", "smtk::resource::Component", "smtk::resource::PersistentObject" }),
    "Failed component hierarchy.");

  // Now test persistent-object methods that use the inheritance tree:
  auto rsrc = smtk::attribute::Resource::create();
  auto def = rsrc->createDefinition("test");
  auto att = rsrc->createAttribute("testA", def);

  test(att->matchesType("smtk::resource::Component"), "Expected attribute to be a component.");
  test(
    !att->matchesType("smtk::graph::Component"), "Expected attribute to not be a graph component.");

  test(rsrc->matchesType("smtk::resource::PersistentObject"), "Expected resource to be an object.");
  test(rsrc->matchesType("smtk::geometry::Resource"), "Expected resource to be a geometry object.");
  test(rsrc->matchesType("smtk::resource::Resource"), "Expected resource to inherit the base.");
  test(!rsrc->matchesType("smtk::model::Resource"), "Expected attribute to not be a model.");

  test(
    validateGenerationsFrom(
      att,
      { { "smtk::attribute::Attribute", 0 },
        { "smtk::resource::Component", 1 },
        { "smtk::resource::PersistentObject", 2 },
        { "Foo", std::string::npos } }),
    "Mismatched component generation counts.");

  test(
    validateGenerationsFrom(
      rsrc,
      { { "smtk::attribute::Resource", 0 },
        { "smtk::geometry::Resource", 1 },
        { "smtk::resource::Resource", 2 },
        { "smtk::resource::PersistentObject", 3 } }),
    "Mismatched resource generation counts.");

  return 0;
}
