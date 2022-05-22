//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_common_StringUtil_h
#define smtk_common_StringUtil_h

#include "smtk/CoreExports.h"

#include <string>
#include <vector>

namespace smtk
{
namespace common
{

class SMTKCORE_EXPORT StringUtil
{
public:
  static std::string& trim(std::string& s);
  static std::string& trimLeft(std::string& s);
  static std::string& trimRight(std::string& s);

  static std::string& lower(std::string& s);
  static std::string& upper(std::string& s);

  static std::vector<std::string>
  split(const std::string& s, const std::string& sep, bool omitEmpty, bool trim);

  /**\brief Converts a string to a boolean
   *
   * If the string \p s can be converted to a boolean then the function returns true
   * and \p value is set to the converted value.  The function will trim \p s and convert it
   * to lower case.  The current acceptable values are: 1, t, true, yes, 0, f, false, no.
   */
  static bool toBoolean(const std::string& s, bool& value);

  /**\brief A comparator for strings that sorts mixed numeric substrings properly.
    *
    * Compare two strings character by character until the first mismatch
    * or one runs out of characters. Upon a mismatch, if both characters
    * have digits, convert to floating-point numbers and choose the return
    * value on whether \a aa's number is less than \a bb (true). Otherwise,
    * the return value is true if \a aa's next character evaluates to less
    * than \a bb's next character.
    *
    * \sa DescriptivePhrase::compareByTitle
    */
  static bool mixedAlphanumericComparator(const std::string& aa, const std::string& bb);
};

} // namespace common
} // namespace smtk

#endif // smtk_common_StringUtil_h
