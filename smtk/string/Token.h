//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_string_Token_h
#define smtk_string_Token_h

#include "smtk/string/Manager.h"

namespace smtk
{
namespace string
{

/** A string token identified by an integer.
  *
  * Often, it is useful for a common, variable-length string to be
  * referenced indirectly rather than storing copies of the string.
  * This class has a static database of hashes; each instance of
  * the class holds only a fixed-size hash value. The string value
  * can be produced in O(1) time by looking the hash up from the
  * static database.
  */
class SMTKCORE_EXPORT Token
{
public:
  /// Construct a token from a string literal.
  Token(const char* data = nullptr, std::size_t size = std::string::npos);
  /// Construct a token from a std::string.
  Token(const std::string& data);

  /// Return the token's ID (usually its hash but possibly not in the case of collisions).
  Hash id() const { return m_id; }
  /// Return the string corresponding to the token.
  const std::string& data() const;

  /// Fast equality comparison (compares hashes, not strings).
  bool operator==(const Token& other) const;
  /// Fast inequality comparison (compares hashes, not strings).
  bool operator!=(const Token& other) const;

  /// Slow, but unsurprising string comparison (preserves lexical string ordering).
  bool operator<(const Token& other) const;
  bool operator>(const Token& other) const;
  bool operator<=(const Token& other) const;
  bool operator>=(const Token& other) const;

  /// Return the database of strings and their tokens (hashes).
  static Manager& manager();

  /// Construct a token from a hash value.
  ///
  /// This method exists for deserialization.
  /// You should avoid using it in other circumstances.
  /// It will throw an exception if the hash does not exist in the manager.
  static Token fromHash(Hash h);

protected:
  Hash m_id;
  static std::shared_ptr<Manager> s_manager;
};

/// Construct a token from a string literal, like so:
///
/// ```c++
/// smtk::string::Token t = """test"""_token
/// std::cout << t.value() << "\n"; // Prints "test"
/// ```
inline Token operator""_token(const char* data, std::size_t size)
{
  return Token{ data, size };
}

} // namespace string
} // namespace smtk

bool SMTKCORE_EXPORT operator==(const std::string& a, const smtk::string::Token& b);
bool SMTKCORE_EXPORT operator!=(const std::string& a, const smtk::string::Token& b);
bool SMTKCORE_EXPORT operator>(const std::string& a, const smtk::string::Token& b);
bool SMTKCORE_EXPORT operator<(const std::string& a, const smtk::string::Token& b);
bool SMTKCORE_EXPORT operator>=(const std::string& a, const smtk::string::Token& b);
bool SMTKCORE_EXPORT operator<=(const std::string& a, const smtk::string::Token& b);

bool SMTKCORE_EXPORT operator==(const smtk::string::Token& a, const std::string& b);
bool SMTKCORE_EXPORT operator!=(const smtk::string::Token& a, const std::string& b);
bool SMTKCORE_EXPORT operator>(const smtk::string::Token& a, const std::string& b);
bool SMTKCORE_EXPORT operator<(const smtk::string::Token& a, const std::string& b);
bool SMTKCORE_EXPORT operator>=(const smtk::string::Token& a, const std::string& b);
bool SMTKCORE_EXPORT operator<=(const smtk::string::Token& a, const std::string& b);

bool SMTKCORE_EXPORT operator==(const char* a, const smtk::string::Token& b);
bool SMTKCORE_EXPORT operator!=(const char* a, const smtk::string::Token& b);
bool SMTKCORE_EXPORT operator>(const char* a, const smtk::string::Token& b);
bool SMTKCORE_EXPORT operator<(const char* a, const smtk::string::Token& b);
bool SMTKCORE_EXPORT operator>=(const char* a, const smtk::string::Token& b);
bool SMTKCORE_EXPORT operator<=(const char* a, const smtk::string::Token& b);

bool SMTKCORE_EXPORT operator==(const smtk::string::Token& a, const char* b);
bool SMTKCORE_EXPORT operator!=(const smtk::string::Token& a, const char* b);
bool SMTKCORE_EXPORT operator>(const smtk::string::Token& a, const char* b);
bool SMTKCORE_EXPORT operator<(const smtk::string::Token& a, const char* b);
bool SMTKCORE_EXPORT operator>=(const smtk::string::Token& a, const char* b);
bool SMTKCORE_EXPORT operator<=(const smtk::string::Token& a, const char* b);

namespace std
{
/// Tokens provide a specialization of std::hash so they can be used in unordered containers.
template<>
struct SMTKCORE_EXPORT hash<smtk::string::Token>
{
  std::size_t operator()(const smtk::string::Token& t) const { return t.id(); }
};
} // namespace std

#endif // smtk_string_Token_h
