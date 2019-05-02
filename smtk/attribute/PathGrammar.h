//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_attribute_PathGrammar_h
#define __smtk_attribute_PathGrammar_h
/*!\file FilterGrammar.h - PEGTL structures for parsing resource filter strings */

#include "tao/pegtl.hpp"
// PEGTL does not itself appear to do anything nasty, but
// on Windows MSVC 2015, it includes something that defines
// a macro named ERROR to be 0. This causes smtkErrorMacro()
// to expand into garbage (because smtk::io::Logger::ERROR
// gets expanded to smtk::io::Logger::0).
#ifdef ERROR
#undef ERROR
#endif

namespace pegtl = tao::pegtl;

namespace smtk
{
namespace attribute
{
namespace pathGrammar
{
struct item_sep : pegtl::string<'/'>
{
};
struct dot : pegtl::one<'.'>
{
};
struct dash : pegtl::one<'-'>
{
};
struct item_identifier : pegtl::plus<pegtl::sor<pegtl::identifier, dot, dash> >
{
}; //This will change in the future
struct leading_identifier : item_identifier
{
}; //This will change in the future
struct residual : pegtl::plus<item_sep, item_identifier>
{
};
struct grammar : pegtl::until<pegtl::eof, item_sep, leading_identifier, pegtl::opt<residual> >
{
};

template <typename Rule>
struct action : pegtl::nothing<Rule>
{
};

template <>
struct action<leading_identifier>
{
  template <typename Input>
  static bool apply(const Input& in, std::string& itemId, std::string& restOfPath, bool& ok)
  {
    itemId = in.string();
    restOfPath.clear();
    ok = true;
    return true;
  }
};
template <>
struct action<residual>
{
  template <typename Input>
  static bool apply(const Input& in, std::string&, std::string& restOfPath, bool& ok)
  {
    restOfPath = in.string();
    ok = true;
    return true;
  }
};
} // namespace pathGrammar
} // namespace attribute
} // namespace smtk

#endif // __smtk_model_FilterGrammar_h
