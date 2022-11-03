//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_Regex_h
#define smtk_Regex_h

#include "smtk/common/CompilerInformation.h"

// We use either STL regex or Boost regex, depending on support. These flags
// correspond to the equivalent logic used to determine the inclusion of Boost's
// regex library.
#if !defined(USE_BOOST_REGEX) &&                                                                   \
  (defined(SMTK_CLANG) ||                                                                          \
   (defined(SMTK_GCC) && __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 9)) ||                \
   defined(SMTK_MSVC))
#include <regex>
namespace smtk
{
using std::regex;
using std::regex_match;
using std::regex_replace;
using std::regex_search;
using std::smatch;
using std::sregex_token_iterator;
} // namespace smtk
#else
SMTK_THIRDPARTY_PRE_INCLUDE
#include <boost/regex.hpp>
SMTK_THIRDPARTY_POST_INCLUDE
namespace smtk
{
using boost::regex;
using boost::regex_match;
using boost::regex_replace;
using boost::regex_search;
using boost::smatch;
using boost::sregex_token_iterator;
} // namespace smtk
#endif

#endif
