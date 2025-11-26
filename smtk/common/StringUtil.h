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

#include <sstream>
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

  /// Return true if and only if \a ss ends with \a ending.
  static bool endsWith(const std::string& ss, const std::string& ending);

  /// Perform in-place replacement of \a search with \a replacement.
  ///
  /// The returned integer is the number of occurrences of \a search in \a source
  /// that were replaced with \a replacement.
  static std::size_t
  replaceAll(std::string& source, const std::string& search, const std::string& replacement);

  /// Perform a single in-place replacement of \a search with \a replacement.
  ///
  /// If \a whichOne is 0, no replacement is performed and std::string::npos is returned.
  ///
  /// If \a whichOne is positive, the n-th occurrence (if it exists) – counting
  /// from the start of \a source – is replaced with \a replacement.
  ///
  /// If \a whichOne is negative, the n-th occurrence (if it exists) – counting
  /// backward from the end of \a source – is replaced with \a replacement.
  ///
  /// If no such occurrence is found, std::string::npos is returned. Otherwise,
  /// the location of the matching occurrence is returned.
  static std::size_t replaceOne(
    std::string& source,
    int whichOne,
    const std::string& search,
    const std::string& replacement);

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

  /**\brief Identify the portion of \a key preceding an underscore followed by an integer number.
    *
    * If the \a key ends with an underscore followed by a (base 10) integer, this method
    * returns true with \a key reduced to the prefix (i.e., the input text
    * up to but not including the underscore) and \a ii set to the numeric value following
    * the underscore. If not, this method returns false and \a ii will be set to 0 while \a key
    * will be unchanged.
    *
    * See UnitTestUniquify for examples.
    */
  static bool endsWithUnderscoreNumber(std::string& key, int& ii);

  /**\brief Return a value of \a src that is unique within the context of \a preExisting
    *       container entries.
    *
    * The returned value will not be any key of the \a preExisting dictionary (whether
    * \a Dict is a set or map). No value is inserted into \a preExisting; if you choose
    * to use the returned value, you are responsible for updating the dictionary.
    */
  template<typename Dict>
  static std::string uniquify(const std::string& src, Dict& preExisting)
  {
    std::string key = src;
    {
      int ii;
      std::string prefix = key;
      StringUtil::endsWithUnderscoreNumber(prefix, ii);
      for (++ii; true; ++ii)
      {
        std::ostringstream ks;
        ks << prefix << "_" << ii;
        key = ks.str();
        if (preExisting.find(key) == preExisting.end())
        {
          break;
        }
      }
    }
    return key;
  }
};

} // namespace common
} // namespace smtk

#endif // smtk_common_StringUtil_h
