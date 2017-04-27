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
#include "smtk/common/RangeDetector.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <iostream>
#include <sstream>

using namespace smtk::common;

template <typename T, typename U>
void checkRanges(T rd, U expected)
{
  std::map<int, int>::const_iterator rdit;
  int ei = 0;
  for (rdit = rd.ranges().begin(); rdit != rd.ranges().end(); ++rdit, ++ei)
  {
    std::ostringstream msg;
    msg << "Expected range " << ei << " to be " << expected[ei][0] << ", " << expected[ei][1]
        << " but it was " << rdit->first << ", " << rdit->second << " instead.";
    test(rdit->first == expected[ei][0] && rdit->second == expected[ei][1], msg.str());
  }
}

int main()
{
  RangeDetector<int> rd;

  test(rd.ranges().empty(), "Ranges not initially empty.");
  std::cout << "Initializing with {1, 2, 3, 5, 6, 7, 11}\n";
  rd.insert(1);
  rd.insert(2);
  rd.insert(3);
  rd.insert(5);
  rd.insert(6);
  rd.insert(7);
  rd.insert(11);
  rd.dump();
  test(rd.ranges().size() == 3, "Expected 3 ranges [1,3], [5,7], [11,11].");
  test(rd.size() == 7, "Expected 7 entries.");
  int expected1[3][2] = { { 1, 3 }, { 5, 7 }, { 11, 11 } };
  checkRanges(rd, expected1);

  std::cout << "Inserting 4\n";
  rd.insert(4);
  rd.dump();
  test(rd.ranges().size() == 2, "Expected 2 ranges [1,7], [11,11].");
  int expected2[2][2] = { { 1, 7 }, { 11, 11 } };
  checkRanges(rd, expected2);

  std::cout << "Inserting 10\n";
  rd.insert(10);
  rd.dump();
  test(rd.ranges().size() == 2, "Expected 2 ranges [1,7], [10,11].");
  int expected3[2][2] = { { 1, 7 }, { 10, 11 } };
  checkRanges(rd, expected3);

  std::cout << "Inserting 8\n";
  rd.insert(8);
  rd.dump();
  test(rd.ranges().size() == 2, "Expected 2 ranges [1,8], [10,11].");
  int expected4[2][2] = { { 1, 8 }, { 10, 11 } };
  checkRanges(rd, expected4);

  std::cout << "Inserting 9\n";
  rd.insert(9);
  rd.dump();
  test(rd.ranges().size() == 1, "Expected 1 range  [1,11].");
  int expected5[1][2] = { { 1, 11 } };
  checkRanges(rd, expected5);

  rd.clear();
  test(rd.ranges().empty(), "Could not reset range detector.");
  return 0;
}
