//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#include "smtk/common/Extension.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <iostream>
#include <set>
#include <string>

using namespace smtk::common;

class TestExtOne : public Extension
{
public:
  smtkTypeMacro(TestExtOne);
  smtkCreateMacro(Extension);
  smtkSuperclassMacro(Extension);
  smtkSharedFromThisMacro(Extension);
  void eval(int num) { std::cout << "Test 1: " << num << "\n"; }
  virtual ~TestExtOne() { std::cout << "    Deleting TestExtOne instance " << this << "\n"; }
protected:
  TestExtOne() { std::cout << "    Creating TestExtOne instance " << this << "\n"; }
};

class TestExtTwo : public Extension
{
public:
  smtkTypeMacro(TestExtTwo);
  smtkCreateMacro(Extension);
  smtkSuperclassMacro(Extension);
  smtkSharedFromThisMacro(Extension);
  void print(int num) { std::cout << "Test 2: " << num << "\n"; }
  virtual ~TestExtTwo() { std::cout << "    Deleting TestExtTwo instance " << this << "\n"; }
protected:
  TestExtTwo() { std::cout << "    Creating TestExtTwo instance " << this << "\n"; }
};

Extension::Ptr TestExtOneCreateBase()
{
  return smtk::dynamic_pointer_cast<Extension>(TestExtOne::create());
}
Extension::Ptr TestExtTwoCreateBase()
{
  return smtk::dynamic_pointer_cast<Extension>(TestExtTwo::create());
}

int main()
{
  test(Extension::findAs<TestExtOne>("test_one") == nullptr, "Found extension before registered.");
  test(Extension::findAs<TestExtTwo>("test_two") == nullptr, "Found extension before registered.");

  Extension::registerExtension("test_one", &TestExtOneCreateBase, false);
  Extension::registerExtension("test_two", &TestExtTwoCreateBase, false);

  auto e1 = Extension::findAs<TestExtOne>("test_one");
  auto e2 = Extension::findAs<TestExtTwo>("test_two");

  test(e1 != nullptr, "Did not find extension one.");
  test(e2 != nullptr, "Did not find extension two.");

  e1->eval(42);
  e2->print(54);

  Extension::unregisterExtension("test_one");
  Extension::unregisterExtension("test_two");

  test(Extension::findAs<TestExtOne>("test_one") == nullptr, "Found extension after unregistered.");
  test(Extension::findAs<TestExtTwo>("test_two") == nullptr, "Found extension after unregistered.");

  Extension::registerExtension("test_1", &TestExtOneCreateBase, true);
  Extension::registerExtension("test_2", &TestExtTwoCreateBase, true);

  Extension::visitAll([](const std::string& name, Extension::Ptr obj) {
    std::cout << "Visiting \"" << name << "\" (" << obj << ")\n";
    // unregister the one-time registration, but don't terminate:
    return std::make_pair(true, false);
  });

  test(Extension::findAs<TestExtOne>("test_1") == nullptr, "Found one-shot extension after use.");
  test(Extension::findAs<TestExtTwo>("test_2") == nullptr, "Found one-shot extension after use.");

  Extension::registerExtension("test_1a", &TestExtOneCreateBase, true);
  Extension::registerExtension("test_1b", &TestExtOneCreateBase, false);
  Extension::registerExtension("test_2a", &TestExtTwoCreateBase, true);
  Extension::registerExtension("test_2b", &TestExtTwoCreateBase, false);

  std::set<std::string> visited;
  std::cout << "Visiting TestExtOne extensions\n";
  Extension::visit<TestExtOne::Ptr>([&visited](const std::string& name, TestExtOne::Ptr t1) {
    visited.insert(name);
    std::cout << name << " ";
    t1->eval(0);
    return std::make_pair(true, false); // unregister if possible, but don't terminate traversal
  });
  std::cout << "Visiting TestExtTwo extensions\n";
  Extension::visit<TestExtTwo::Ptr>([&visited](const std::string& name, TestExtTwo::Ptr t2) {
    visited.insert(name);
    std::cout << name << " ";
    t2->print(1);
    return std::make_pair(true, false); // unregister if possible, but don't terminate traversal
  });

  test(visited.find("test_1a") != visited.end(), "Did not visit 1a");
  test(visited.find("test_1b") != visited.end(), "Did not visit 1b");
  test(visited.find("test_2a") != visited.end(), "Did not visit 2a");
  test(visited.find("test_2b") != visited.end(), "Did not visit 2b");

  test(Extension::findAs<TestExtOne>("test_1a") == nullptr, "Found one-shot extension after use.");
  test(Extension::findAs<TestExtTwo>("test_2a") == nullptr, "Found one-shot extension after use.");
  test(Extension::findAs<TestExtOne>("test_1b") != nullptr, "Missing extension 1b after use.");
  test(Extension::findAs<TestExtTwo>("test_2b") != nullptr, "Missing extension 2b after use.");

  return 0;
}
