//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_common_URL_h
#define smtk_common_URL_h

#include "smtk/string/Token.h"

namespace smtk
{
namespace common
{

/** An RFC3986-compliant Uniform Resource Locator (URL).
  *
  * This class holds URL data as a set of string tokens.
  * If given a single string, that string is parsed and
  * decomposed into tokens for querying data.
  */
class SMTKCORE_EXPORT URL
{
public:
  using Token = smtk::string::Token;

  URL() = default;
  URL(const URL& other) = default;
  URL(const std::string& txt);
  URL(Token scheme, Token authority, Token path, Token query = Token(), Token fragment = Token());

  bool valid() const;
  bool operator!=(URL const& other) const;
  bool operator==(URL const& other) const;
  bool operator<(URL const& other) const;

  URL& operator=(URL const& other) = default;

  operator bool() const;
  explicit operator std::string() const;

  ///@{
  /// Set/get various elements of the URL.
  ///
  /// See [rfc3896](https://www.rfc-editor.org/rfc/rfc3986) for a detailed
  /// description of each element in a URL.
  Token scheme() const { return m_scheme; }
  bool setScheme(Token scheme);

  Token authority() const { return m_authority; }
  bool setAuthority(Token authority);

  Token path() const { return m_path; }
  bool setPath(Token path);

  Token query() const { return m_query; }
  bool setQuery(Token query);

  Token fragment() const { return m_fragment; }
  bool setFragment(Token fragment);
  ///@}

protected:
  Token m_scheme;
  Token m_authority;
  Token m_path;
  Token m_query;
  Token m_fragment;
};

SMTKCORE_EXPORT std::ostream& operator<<(std::ostream& stream, const URL& url);

} // namespace common
} // namespace smtk

#endif // smtk_common_URL_h
