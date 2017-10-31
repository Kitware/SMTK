//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_common_testing_cxx_helpers_h
#define smtk_common_testing_cxx_helpers_h

#include <iostream>
#include <sstream>
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

/// A macro for running tests with more verbose output messages.
#define smtkTest(condition, msg)                                                                   \
  {                                                                                                \
    std::ostringstream explanation;                                                                \
    explanation << msg;                                                                            \
    test(condition, explanation.str());                                                            \
  }

#endif
