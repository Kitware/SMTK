//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/common/UnionFind.h"

#include <iostream>

#include "smtk/common/testing/cxx/helpers.h"

using namespace smtk::common;

template<typename T>
int testUnionFind()
{
  UnionFind<T> uf;
  test(uf.size() == 0, "Expected an initially-empty set.");
  test(uf.find(0) == -1, "Expected -1 when calling Find() on an invalid set.");

  T s0 = uf.newSet();
  T s1 = uf.newSet();
  T s2 = uf.newSet();
  test(uf.size() == 3, "Expected a set of size 3.");

  test(uf.find(s0) >= 0, "Expected a valid set ID for s0.");
  test(uf.find(s1) >= 0, "Expected a valid set ID for s1.");
  test(uf.find(s2) >= 0, "Expected a valid set ID for s2.");
  test(uf.find(s0) != uf.find(s1), "Expected s0 and s1 to be initially disjoint.");
  test(uf.find(s0) != uf.find(s2), "Expected s0 and s2 to be initially disjoint.");

  T s3 = uf.mergeSets(s0, s1);

  test(uf.find(s0) == uf.find(s1), "Expected s0 and s1 to match after merge.");
  test(uf.find(s0) == uf.find(s3), "Expected s0 and s3 to match after merge.");
  test(uf.find(s1) == uf.find(s3), "Expected s0 and s3 to match after merge.");

  test(uf.find(s0) != uf.find(s2), "Expected s0 and s2 to be disjoint after merge.");
  test(uf.find(s3) != uf.find(s2), "Expected s3 and s2 to be disjoint after merge.");

  typename std::map<T,T> collapse;
  uf.collapseIds(collapse, 100);

  std::cout
    << static_cast<int>(s0) << " -> " << static_cast<int>(collapse[uf.find(s0)]) << "\n"
    << static_cast<int>(s1) << " -> " << static_cast<int>(collapse[uf.find(s1)]) << "\n"
    << static_cast<int>(s2) << " -> " << static_cast<int>(collapse[uf.find(s2)]) << "\n"
    << "\n"
    ;

  test(collapse[uf.find(s0)] == 100, "Invalid collapsed ID for s0.");
  test(collapse[uf.find(s1)] == 100, "Invalid collapsed ID for s1.");
  test(collapse[uf.find(s2)] == 101, "Invalid collapsed ID for s2.");
  test(collapse.size() == uf.roots().size(), "Expected a collapsed-ID map of size 2.");
  test(uf.size() == 3, "Still expected a set of size 3.");

  return 0;
}

int main(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  return
    testUnionFind<int>() ||
    testUnionFind<signed char>();
}
