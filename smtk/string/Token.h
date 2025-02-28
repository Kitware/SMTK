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
  /// Construct a token given its hash value.
  /// NOTE: This will NOT insert a string into the manager as other constructors do.
  constexpr Token(Hash tokenId) noexcept
    : m_id(tokenId)
  {
  }

  /// An invalid hash, used with the constructor above to make an invalid token.
  static constexpr Hash Invalid = Manager::Invalid;

  /// Return the token's ID (usually its hash but possibly not in the case of collisions).
  Hash id() const { return m_id; }
  /// Return the string corresponding to the token.
  const std::string& data() const;
  /// Return true if a string corresponding to the token exists.
  bool hasData() const;
  /// Return true if the token's ID has been set (false if not).
  bool valid() const;

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

protected:
  // ----
  // Adapted from https://notes.underscorediscovery.com/constexpr-fnv1a/index.html
  // which declared the source as public domain or equivalent. Retrieved on 2022-07-22.
  // See also https://gist.github.com/ruby0x1/81308642d0325fd386237cfa3b44785c .
  static constexpr uint32_t hash32a_const = 0x811c9dc5;
  static constexpr uint32_t hash32b_const = 0x1000193;
  static constexpr uint64_t hash64a_const = 0xcbf29ce484222325;
  static constexpr uint64_t hash64b_const = 0x100000001b3;

  // Compute a 32-bit hash of a string.
  // Unlike the original, this version handles embedded null characters so that
  // unicode multi-byte sequences can be hashed.
  template<typename T>
  static constexpr typename std::enable_if<sizeof(T) == 4, T>::type
  hash_fnv1a_const(const char* str, std::size_t size, T value) noexcept
  {
    return (!str || size <= 0)
      ? value
      : hash_fnv1a_const<T>(&str[1], size - 1, (value ^ uint32_t(str[0])) * hash32b_const);
  }

  template<typename T>
  static constexpr typename std::enable_if<sizeof(T) == 4, std::size_t>::type hash_fnv1a_seed()
  {
    return hash32a_const;
  }

  // Compute a 64-bit hash of a string.
  template<typename T>
  static constexpr typename std::enable_if<sizeof(T) == 8, std::size_t>::type
  hash_fnv1a_const(const char* str, std::size_t size, uint64_t value) noexcept
  {
    return (!str || size <= 0)
      ? value
      : hash_fnv1a_const<T>(&str[1], size - 1, (value ^ uint64_t(str[0])) * hash64b_const);
  }

  template<typename T>
  static constexpr typename std::enable_if<sizeof(T) == 8, std::size_t>::type hash_fnv1a_seed()
  {
    return hash64a_const;
  }

public:
  /// Return the hash of a string
  /// This is used internally but also by the ""_token() literal operator
  static constexpr Hash stringHash(const char* data, std::size_t size) noexcept
  {
    return Token::hash_fnv1a_const<std::size_t>(data, size, Token::hash_fnv1a_seed<std::size_t>());
  }

  /// Construct a token given only its hash.
  /// This variant checks that the string manager holds data for the hash and
  /// will throw an exception if it does not.
  static Token fromHash(Hash h);

protected:
  Hash m_id;
  static std::shared_ptr<Manager> s_manager;
};

namespace literals
{

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

constexpr Hash operator""_hash(const char* data, std::size_t size)
{
  return Token::stringHash(data, size);
}

} // namespace literals
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
