//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/common/UUID.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <iostream>
#include <sstream>

using smtk::common::UUID;

int main(int argc, char* argv[])
{
  (void)argc;
  (void)argv;

  // Default constructor.
  UUID a;
  test(a.isNull(), "Empty constructor must create NULL UUID");
  test(a.toString() == "00000000-0000-0000-0000-000000000000", "ToString(NULL)");

  // Raw data constructor
  UUID::value_type data[] = "\x00\x00\xff\xff\x00\x00\xff\xff\x00\x00\xff\xff\x00\x00\xff\xff";
  UUID fromRaw(data, data + 16);

  // String constructor
  UUID fromStr("a3d75703-fc9b-4d99-a104-ee67cf6d11b9");

  // Try the << operator
  std::ostringstream os;
  os << fromRaw;
  test(fromRaw.toString() == os.str(), "operator << failed");

  // Try the >> operator:
  std::istringstream is(fromStr.toString());
  is >> fromRaw;
  test(fromRaw == fromStr, "operator >> failed");

  // Test comparators
  UUID b(std::string("a3d75703-fc9b-4d99-a104-ee67cf6d11b9"));
  UUID c(std::string("5f0dee12-8b03-46dd-af36-ec8f9ca33882"));
  UUID d(std::string("5f0dee12-8b03-46dd-af36-ec8f9ca33882"));
  test(c < b, "Less-than operator failed (TRUE)");
  test(!(c < d), "Less-than operator failed (FALSE)");
  test(b != c, "Inequality operator failed (TRUE)");
  test(!(c != c), "Inequality operator failed (FALSE)");
  test(c == d, "Equality operator failed (TRUE)");
  test(!(b == c), "Equality operator failed (FALSE)");

  // Generators:
  UUID e = UUID::random();
  UUID f = UUID::null();
  test(!e.isNull(), "random() constructor must not create NULL UUID");
  test(f.isNull(), "null() constructor must create NULL UUID");

  // Test casting to a boolean.
  test(!f, "Cast of null UUID to boolean should be false");
  test(b, "Cast of non-null UUID to boolean should be true");

  return 0;
}
