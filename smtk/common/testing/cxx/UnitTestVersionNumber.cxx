//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/common/VersionNumber.h"
#include "smtk/common/json/jsonVersionNumber.h"
#include "smtk/common/testing/cxx/helpers.h"

#include "nlohmann/json.hpp"

#include <iostream>

using namespace smtk::common;

int UnitTestVersionNumber(int /*unused*/, char** const /*unused*/)
{
  VersionNumber unassigned;
  test(!unassigned.isValid(), "Default constructor should provide an invalid version.");

  VersionNumber v3("3.0");
  VersionNumber v4(4, 0, 0);
  VersionNumber a = v3;
  test(v3.isValid(), "String constructor should provide a valid version.");
  test(v4.isValid(), "Integer constructor should provide a valid version.");
  test(a.isValid(), "Copy constructor should provide a valid version.");
  test(a == v3, "Copy assignment failed.");

  VersionNumber b;
  nlohmann::json j1 = "4.0.0";
  b = j1;
  test(b == v4, "JSON deserialization failed.");

  nlohmann::json j2 = v3;
  VersionNumber c = j2;
  test(v3 == c, "JSON round trip failed.");

  // Verify that a leading zero does not cause problems
  // for month-based release numbers in september (which
  // have caused trouble in the past due to "09" being
  // treated as octal (because of the leading zero) and
  // thus failing conversion.
  VersionNumber yearMonth("21.09.11");
  std::cout << "ym = " << yearMonth << "\n";
  test(yearMonth == VersionNumber(21, 9, 11), "Improper decoding of year+month version number.");

  std::cout << "v3 = " << v3 << "\n";
  std::cout << "v4 = " << v4 << "\n";
  test(v3 < v4, "Unexpected ordering of version numbers.");
  test(v4 > v3, "Unexpected ordering of version numbers.");
  test(!(v4 < v3), "Unexpected ordering of version numbers.");

  return 0;
}
