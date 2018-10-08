//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include <iostream>
#include <typeinfo>

#include "smtk/mesh/core/Handle.h"

#include "smtk/mesh/json/jsonHandleRange.h"

using nlohmann::json;

int UnitTestIntervals(int, char** const)
{
  using namespace smtk::mesh;

  HandleInterval interval(6, 8);

  HandleRange range1;
  range1.insert(2);
  range1.insert(3);
  range1.insert(interval);

  HandleRange range2;
  range2.insert(12);
  range2.insert(5);

  std::cout << "range 1: " << range1 << std::endl;
  std::cout << "range 2: " << range2 << std::endl;
  std::cout << "range 1 - range 2: " << (range1 - range2) << std::endl;
  std::cout << "range 2 - range 1: " << (range2 - range1) << std::endl;
  std::cout << "union(range 1, range 2): " << (range1 | range2) << std::endl;
  std::cout << "intersection(range 1, range 2): " << (range1 & range2) << std::endl;
  std::cout << "range 1 size: " << range1.size() << std::endl;
  std::cout << "range 1 interval count: " << rangeIntervalCount(range1) << std::endl;
  std::cout << "range 1 empty: " << range1.empty() << std::endl;
  std::cout << "range 1 - 3: " << (range1 - Handle(3)) << std::endl;
  std::cout << "range 1 - (3,4): " << (range1 - HandleInterval(3, 4)) << std::endl;
  std::cout << "range 1 contains 3: " << rangeContains(range1, Handle(3)) << std::endl;
  std::cout << "range 1 contains (2,4): " << rangeContains(range1, HandleInterval(2, 4))
            << std::endl;
  std::cout << "range 1 contains (0,4): " << rangeContains(range1, HandleInterval(0, 4))
            << std::endl;
  std::cout << "(range 1)[0]: " << rangeElement(range1, 0) << std::endl;
  std::cout << "(range 1)[3]: " << rangeElement(range1, 4) << std::endl;

  nlohmann::json j = range1;
  std::cout << j.dump() << std::endl;

  HandleRange range3 = j;
  std::cout << range3 << std::endl;

  return 0;
}
