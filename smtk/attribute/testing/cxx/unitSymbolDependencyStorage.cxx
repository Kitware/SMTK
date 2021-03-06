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

#include "smtk/attribute/SymbolDependencyStorage.h"

#include "smtk/common/testing/cxx/helpers.h"

void testPreventsAddingCyclicDependency()
{
  smtk::attribute::SymbolDependencyStorage storage;
  storage.addDependency("a", "b");
#ifndef NDEBUG
  storage.dump();
#endif
  smtkTest(storage.isDependentOn("a", "b") == true, "a should be dependent on b")
    smtkTest(storage.addDependency("b", "a") == false, "b cannot be dependent on a")
}

void testGetsAllDependentExpressions()
{
  smtk::attribute::SymbolDependencyStorage storage;
  storage.addDependency("a", "b");
  storage.addDependency("b", "c");
  storage.addDependency("b", "z");
#ifndef NDEBUG
  storage.dump();
#endif
  smtkTest((storage.allDependentSymbols("a") == std::vector<std::string>{ "b", "c", "z" }),
    "b, c, and z are dependent on a")
    smtkTest((storage.allDependentSymbols("b") == std::vector<std::string>{ "c", "z" }),
      "c and z are dependent on b")
}

void testPrunesOldSymbols()
{
  smtk::attribute::SymbolDependencyStorage storage;
  storage.addDependency("y", "z"); // z depends on y.
  storage.addDependency("f", "z"); // z depends on f.

  storage.pruneOldSymbols(std::unordered_set<std::string>{ "f" }, "z");
#ifndef NDEBUG
  storage.dump();
#endif
  smtkTest(storage.isDependentOn("y", "z") == false, "z should not depend on y anymore")
    smtkTest(storage.isDependentOn("f", "z") == true, "z should be dependent on f")
}

void testIsDependentOn()
{
  smtk::attribute::SymbolDependencyStorage storage;
  smtkTest(storage.isDependentOn("foo", "bar") == false, "neither foo nor bar is present")
    storage.addDependency("foo", "bar");
  storage.addDependency("bar", "baz");
  smtkTest(storage.isDependentOn("foo", "baz") == true, "expected foo to be dependent on baz")
    smtkTest(storage.isDependentOn("baz", "foo") == false, "baz should not be dependent on foo")
}

int unitSymbolDependencyStorage(int /*argc*/, char** const /*argv*/)
{
  testPreventsAddingCyclicDependency();
  testGetsAllDependentExpressions();
  testPrunesOldSymbols();
  testIsDependentOn();
  return 0;
}
