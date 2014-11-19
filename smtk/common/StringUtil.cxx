#ifndef SHIBOKEN_SKIP
#include "smtk/common/StringUtil.h"

#include "smtk/Function.h" // for smtk::bind

#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

using namespace smtk::placeholders;

namespace smtk {
  namespace common {

static std::locale safeLocale = std::locale::classic();

/// Trim whitespace from both ends of a string (in place).
std::string& StringUtil::trim(std::string& s)
{
  return StringUtil::trimLeft(
    StringUtil::trimRight(s));
}

/// Trim whitespace from start of string (in place).
std::string& StringUtil::trimLeft(std::string& s)
{
  s.erase(
    s.begin(),
    std::find_if(
      s.begin(), s.end(),
      std::not1(
        std::ptr_fun<int, int>(
          isspace //smtk::bind(std::isspace<char>, _1, safeLocale)
        ))));
  return s;
}

/// Trim whitespace from end of string (in place).
std::string& StringUtil::trimRight(std::string& s)
{
  s.erase(
    std::find_if(
      s.rbegin(), s.rend(),
      std::not1(
        std::ptr_fun<int, int>(
          isspace //smtk::bind(std::isspace<char>, _1, safeLocale)
        ))).base(),
    s.end());
  return s;
}

/// Transform string to all-lowercase letters (in place).
std::string& StringUtil::lower(std::string& s)
{
  std::transform(
    s.begin(), s.end(), s.begin(),
    smtk::bind(std::tolower<char>, _1, safeLocale));
  return s;
}

/// Transform string to all-uppercase letters (in place).
std::string& StringUtil::upper(std::string& s)
{
  std::transform(
    s.begin(), s.end(), s.begin(),
    smtk::bind(std::toupper<char>, _1, safeLocale));
  return s;
}

/// Split string into a vector of strings at each occurrence of \a sep (including or omitting empty strings at double-separators)
std::vector<std::string> StringUtil::split(
  const std::string& s, const std::string& sep,
  bool omitEmpty, bool trim)
{
  std::vector<std::string> result;
  std::size_t start;
  std::size_t match;
  for (start = 0; start != std::string::npos; )
    {
    match = s.find_first_of(sep, start);
    std::string token = s.substr(start, match);
    if (trim)
      StringUtil::trim(token);
    if (!omitEmpty || !token.empty())
      result.push_back(token);
    start = (match == std::string::npos ? match : match + 1);
    }
  return result;
}

  } // namespace common
} // namespace smtk

#endif // SHIBOKEN_SKIP
