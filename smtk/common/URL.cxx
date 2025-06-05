//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/common/URL.h"

#include "tao/pegtl.hpp"
#include "tao/pegtl/contrib/uri.hpp"

namespace pegtl = tao::pegtl;

namespace smtk
{
namespace common
{
namespace uri
{

template<bool (smtk::common::URL::*Field)(smtk::string::Token)>
struct bind
{
  template<typename Input>
  static void apply(const Input& in, smtk::common::URL& url)
  {
    (url.*Field)(smtk::string::Token(in.string()));
  }
};

// clang-format off
template< typename Rule > struct action {};

template<> struct action< pegtl::uri::scheme > : bind< &URL::setScheme > {};
template<> struct action< pegtl::uri::authority > : bind< &URL::setAuthority > {};
// template<> struct action< pegtl::uri::host > : bind< &URL::setHost > {};
// template<> struct action< pegtl::uri::port > : bind< &URL::setPort > {};
template<> struct action< pegtl::uri::path_noscheme > : bind< &URL::setPath > {};
template<> struct action< pegtl::uri::path_rootless > : bind< &URL::setPath > {};
template<> struct action< pegtl::uri::path_absolute > : bind< &URL::setPath > {};
template<> struct action< pegtl::uri::path_abempty > : bind< &URL::setPath > {};
template<> struct action< pegtl::uri::query > : bind< &URL::setQuery > {};
template<> struct action< pegtl::uri::fragment > : bind< &URL::setFragment > {};
// clang-format on

} // namespace uri

URL::URL(const std::string& txt)
{
  using grammar = pegtl::must<pegtl::uri::URI>;
  pegtl::memory_input<> input(txt, "smtk::common::URL");
  pegtl::parse<grammar, uri::action>(input, *this);
}

URL::URL(Token scheme, Token authority, Token path, Token query, Token fragment)
  : m_scheme(scheme)
  , m_authority(authority)
  , m_path(path)
  , m_query(query)
  , m_fragment(fragment)
{
}

bool URL::valid() const
{
  // The URL "/" is not valid. If you want the root of the local filesystem, use "file:///".
  return m_path.valid() || m_scheme.valid();
}

bool URL::setScheme(Token scheme)
{
  if (scheme == m_scheme)
  {
    return false;
  }
  m_scheme = scheme;
  return true;
}

bool URL::setAuthority(Token authority)
{
  if (authority == m_authority)
  {
    return false;
  }
  m_authority = authority;
  return true;
}

bool URL::setPath(Token path)
{
  if (path == m_path)
  {
    return false;
  }
  m_path = path;
  return true;
}

bool URL::setQuery(Token query)
{
  if (query == m_query)
  {
    return false;
  }
  m_query = query;
  return true;
}

bool URL::setFragment(Token fragment)
{
  if (fragment == m_fragment)
  {
    return false;
  }
  m_fragment = fragment;
  return true;
}

bool URL::operator!=(URL const& other) const
{
  return m_scheme != other.m_scheme || m_authority != other.m_authority || m_path != other.m_path ||
    m_query != other.m_query || m_fragment != other.m_fragment;
}

bool URL::operator==(URL const& other) const
{
  return m_scheme == other.m_scheme && m_authority == other.m_authority && m_path == other.m_path &&
    m_query == other.m_query && m_fragment == other.m_fragment;
}

bool URL::operator<(URL const& other) const
{
  return m_scheme < other.m_scheme ||
    (m_scheme == other.m_scheme &&
     (m_authority < other.m_authority ||
      (m_authority == other.m_authority &&
       (m_path < other.m_path ||
        (m_path == other.m_path &&
         (m_query < other.m_query ||
          (m_query == other.m_query && m_fragment < other.m_fragment)))))));
}

URL::operator bool() const
{
  return this->valid();
}

URL::operator std::string() const
{
  std::string result;
  if (m_scheme.valid())
  {
    result += m_scheme.data() + ":";
  }
  if (m_authority.valid())
  {
    result += "//" + m_authority.data();
  }
  // result += "/";
  if (m_path.valid())
  {
    result += m_path.data();
  }
  if (m_query.valid())
  {
    result += "?" + m_query.data();
  }
  if (m_fragment.valid())
  {
    result += "#" + m_fragment.data();
  }
  return result;
}

std::ostream& operator<<(std::ostream& stream, const URL& url)
{
  std::string data = (std::string)url;
  stream << data;
  return stream;
}

} // namespace common
} // namespace smtk
