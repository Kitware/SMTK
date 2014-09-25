//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_common_testing_helpers_h
#define __smtk_common_testing_helpers_h

#include <iostream>
#include <string>

/**\brief A function for unit tests that behaves like assert.
  *
  * When \a condition is non-zero, the test passes \a condition's
  * value as its output. Otherwise it throws an exception containing
  * the optional message string \a explanation.
  *
  * While this function behaves like assert, it doesn't get optimized
  * away in release builds.
  * Use this instead of assert to avoid getting "unused variable"
  * warnings in unit tests.
  */
inline int test(int condition, const std::string& explanation = std::string())
{
  if (!condition)
    {
    if (!explanation.empty())
      {
      std::cerr << "## TEST FAILURE ##\n\n  " << explanation << "\n\n## TEST FAILURE ##\n";
      }
    throw explanation;
    }
  return condition;
}

#endif // __smtk_common_testing_helpers_h
