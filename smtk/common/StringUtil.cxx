//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/common/StringUtil.h"

#include "smtk/Function.h" // for smtk::bind

#include <algorithm>
#include <cctype>
#include <functional>
#include <locale>
#include <set>

using namespace smtk::placeholders;

namespace smtk
{
namespace common
{

static std::locale safeLocale = std::locale::classic();

// Some awesome whitespace trimmers based on
// http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
//
/// Trim whitespace from both ends of a string (in place).
std::string& StringUtil::trim(std::string& s)
{
  return StringUtil::trimLeft(StringUtil::trimRight(s));
}

/// Trim whitespace from start of string (in place).
std::string& StringUtil::trimLeft(std::string& s)
{
  s.erase(
    s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char c) { return !std::isspace(c); }));
  return s;
}

/// Trim whitespace from end of string (in place).
std::string& StringUtil::trimRight(std::string& s)
{
  s.erase(
    std::find_if(s.rbegin(), s.rend(), [](unsigned char c) { return !std::isspace(c); }).base(),
    s.end());
  return s;
}

/// Transform string to all-lowercase letters (in place).
std::string& StringUtil::lower(std::string& s)
{
  std::transform(
    s.begin(),
    s.end(),
    s.begin(),
    smtk::bind(std::tolower<char>, std::placeholders::_1, safeLocale));
  return s;
}

/// Transform string to all-uppercase letters (in place).
std::string& StringUtil::upper(std::string& s)
{
  std::transform(
    s.begin(),
    s.end(),
    s.begin(),
    smtk::bind(std::toupper<char>, std::placeholders::_1, safeLocale));
  return s;
}

/// Split string into a vector of strings at each occurrence of \a sep (including or omitting empty strings at double-separators)
std::vector<std::string>
StringUtil::split(const std::string& s, const std::string& sep, bool omitEmpty, bool trim)
{
  std::vector<std::string> result;
  std::size_t start;
  std::size_t match;
  for (start = 0; start != std::string::npos;)
  {
    match = s.find_first_of(sep, start);
    std::string token = s.substr(start, match - start);
    if (trim)
      StringUtil::trim(token);
    if (!omitEmpty || !token.empty())
      result.push_back(token);
    start = (match == std::string::npos ? match : match + 1);
  }
  return result;
}

bool StringUtil::toBoolean(const std::string& s, bool& value)
{
  const std::set<std::string> trueValues = { "1", "t", "true", "yes" };
  const std::set<std::string> falseValues = { "0", "f", "false", "no" };

  std::string temp = s;
  StringUtil::lower(StringUtil::trim(temp));

  auto it = trueValues.find(temp);
  if (it != trueValues.end())
  {
    value = true;
    return true;
  }
  it = falseValues.find(temp);
  if (it != falseValues.end())
  {
    value = false;
    return true;
  }
  return false;
}

bool StringUtil::mixedAlphanumericComparator(const std::string& aa, const std::string& bb)
{
  if (aa.empty() && bb.empty())
  {
    return false;
  }

  if (aa.empty())
  {
    return true;
  }

  if (bb.empty())
  {
    return false;
  }

  std::string::size_type minlen = aa.size() < bb.size() ? aa.size() : bb.size();
  std::string::size_type i;
  for (i = 0; i < minlen; ++i)
    if (aa[i] != bb[i])
      break; // Stop at the first difference between aa and bb.

  // Shorter strings are less than longer versions with the same start:
  if (i == minlen)
    return aa.size() < bb.size();

  // Both aa & bb have some character present and different.
  bool da = isdigit(aa[i]) != 0;
  bool db = isdigit(bb[i]) != 0;
  if (da && !db)
    return true; // digits come before other things
  if (!da && db)
    return false; // non-digits come after digits
  if (!da && !db)
    return aa[i] < bb[i];
  // Now, both aa and bb differ with some numeric value.
  // Convert to numbers and compare the numbers.
  double na = atof(aa.substr(i).c_str());
  double nb = atof(bb.substr(i).c_str());
  return na < nb;
}
} // namespace common
} // namespace smtk
