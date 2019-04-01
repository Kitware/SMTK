//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_FilterGrammar_h
#define __smtk_model_FilterGrammar_h
/*!\file FilterGrammar.h - PEGTL structures for parsing resource filter strings */

#include "smtk/model/LimitingClause.h"

#include "tao/pegtl.hpp"
// PEGTL does not itself appear to do anything nasty, but
// on Windows MSVC 2015, it includes something that defines
// a macro named ERROR to be 0. This causes smtkErrorMacro()
// to expand into garbage (because smtk::io::Logger::ERROR
// gets expanded to smtk::io::Logger::0).
#ifdef ERROR
#undef ERROR
#endif

namespace smtk
{
namespace model
{

using namespace tao::pegtl;

// clang-format off
// -----
// This fragment schnorked shamelessly from PEGTL's example double:
// Copyright (c) 2014-2018 Dr. Colin Hirsch and Daniel Frey
// Visit https://github.com/taocpp/PEGTL/ for license information.
struct plus_minus : opt< one< '+', '-' > > {};
struct dot : one< '.' > {};
struct inf : seq< TAO_PEGTL_ISTRING("inf"),
                  opt< TAO_PEGTL_ISTRING("inity") > > {};
struct nan : seq< TAO_PEGTL_ISTRING("nan"),
                  opt< one< '(' >,
                       plus< alnum >,
                       one< ')' > > > {};
template< typename D >
struct number :
  if_then_else< dot,
    plus< D >,
    seq< plus< D >, opt< dot, star< D > > >
  > {};
struct e : one< 'e', 'E' > {};
struct p : one< 'p', 'P' > {};
struct exponent : seq< plus_minus, plus< digit > > {};
struct decimal : seq< number< digit >, opt< e, exponent > > {};
struct hexadecimal : seq< one< '0' >, one< 'x', 'X' >, number< xdigit >, opt< p, exponent > > {};
struct fpnumber : seq< plus_minus, sor< hexadecimal, decimal, inf, nan > > {};
// -----

// The following declarations specify a grammar for limiting clauses
// on model-entity search strings (i.e., arguments passed to
// the smtk::model::Resource::queryOperation() method).

// A. String literals.
struct open_square : string<'['> {};
struct close_square : string<']'> {};
struct prop_int : TAO_PEGTL_ISTRING("integer") {};
struct prop_float : TAO_PEGTL_ISTRING("floating-point") {};
struct prop_string : TAO_PEGTL_ISTRING("string") {};
struct open_curly : string<'{'> {};
struct close_curly : string<'}'> {};
struct open_paren : string<'('> {};
struct close_paren : string<')'> {};
struct comma : string<','> {};
struct equal : string<'='> {};
struct optspace : opt<plus<one<' ', '\t', '\n'> > > {};

template<char Quote>
struct quoted_string
{
  template<typename Input>
  static bool match(Input& in)
  {
    if (!in.empty())
    {
      auto start = *in.current();
      if (start == Quote)
      {
        in.bump(1);
        auto curr = *in.current();
        // Bump to the end of the string or return false if the terminator is unmatched.
        while (curr != start)
        {
          in.bump(1);
          if (in.empty())
          {
            return false;
          }
          curr = *in.current();
          if (curr == '\\')
          { // skip the character after the escape.
            in.bump(2);
            if (in.empty())
            {
              return false;
            }
            curr = *in.current();
          }
        }
        if (in.empty())
        {
          return false;
        }
        if (*in.current() == start)
        {
          in.bump(1);
          return true;
        }
        return false;
      }
    }
    return false;
  }
};

// B. Combinations, sequences, identifiers.
struct prop_type : sor< prop_int, prop_float, prop_string > {};
struct prop_name : quoted_string<'\''> {};
struct prop_string_value : prop_name {}; // Tokenization is identical with prop_name above.
struct prop_name_regex : quoted_string<'/'> {};
struct prop_string_regex : quoted_string<'/'> {}; // Tokenization is identical with prop_name_regex above.
struct prop_int_value : plus< digit > {};
struct prop_fp_value : fpnumber {};

struct prop_fpn_seq :
  sor<
    prop_fp_value,
    seq<
      open_paren,
      optspace,
      prop_fp_value,
      optspace,
      opt<
        plus<
          seq<
            comma,
            optspace,
            prop_fp_value,
            optspace
          >
        >
      >,
      close_paren
    >
  > {};

struct prop_int_seq :
  sor<
    prop_int_value,
    seq<
      open_paren,
      optspace,
      prop_int_value,
      optspace,
      opt<
        plus<
          seq<
            comma,
            optspace,
            prop_int_value,
            optspace
          >
        >
      >,
      close_paren
    >
  > {};

struct prop_str_seq :
  sor<
    prop_string_value,
    prop_string_regex,
    seq<
      open_paren,
      optspace,
      prop_string_value,
      optspace,
      opt<
        plus<
          seq<
            comma,
            optspace,
            sor< prop_string_regex, prop_string_value >,
            optspace
          >
        >
      >,
      close_paren
    >,
    seq<
      open_paren,
      optspace,
      prop_string_regex,
      optspace,
      opt<
        plus<
          seq<
            comma,
            optspace,
            sor< prop_string_regex, prop_string_value >,
            optspace
          >
        >
      >,
      close_paren
    >
  > {};

// The total grammar for limiting clauses:
// we accept any of 3 sequences (one for string-property limiters, one
// for floating-point-property limiters, one for integer-property limiters)
struct SMTKCORE_EXPORT FilterGrammar :
  seq<
    optspace,
    sor<
      seq<
        open_square,
        optspace,
        prop_float,
        optspace,
        opt<
          seq<
            open_curly,
            optspace,
            sor< prop_name, prop_name_regex >,
            optspace,
            opt< equal, optspace, prop_fpn_seq>,
            optspace,
            close_curly,
            optspace
          >
        >,
        close_square
      >,
      seq<
        open_square,
        optspace,
        prop_int,
        optspace,
        opt<
          seq<
            open_curly,
            optspace,
            sor< prop_name, prop_name_regex >,
            optspace,
            opt<
              seq<
                equal,
                optspace,
                prop_int_seq,
                optspace
              >
            >,
            close_curly
          >
        >,
        close_square
      >,
      seq<
        open_square,
        optspace,
        prop_string,
        optspace,
        opt<
          seq<
            open_curly,
            optspace,
            sor< prop_name, prop_name_regex >,
            optspace,
            opt<
              seq<
                equal,
                optspace,
                prop_str_seq,
                optspace
              >
            >,
            close_curly
          >
        >,
        close_square
      >
    >
  > {};

// clang-format on

// Actions on the state in response to encountered grammar.
template <typename Rule>
struct SMTKCORE_EXPORT FilterAction : nothing<Rule>
{
};

template <>
struct FilterAction<prop_int>
{
  template <typename Input>
  static void apply(const Input&, LimitingClause& clause)
  {
    clause.m_propType = smtk::resource::PropertyType::INTEGER_PROPERTY;
  }
};

template <>
struct FilterAction<prop_float>
{
  template <typename Input>
  static void apply(const Input&, LimitingClause& clause)
  {
    clause.m_propType = smtk::resource::PropertyType::FLOAT_PROPERTY;
  }
};

template <>
struct FilterAction<prop_string>
{
  template <typename Input>
  static void apply(const Input&, LimitingClause& clause)
  {
    clause.m_propType = smtk::resource::PropertyType::STRING_PROPERTY;
  }
};

template <>
struct FilterAction<prop_name>
{
  template <typename Input>
  static void apply(const Input& in, LimitingClause& clause)
  {
    auto pname = in.string(); // This has single quotes around it.
    // std::cout << "prop_name <" << pname << "> plain\n";
    clause.m_propName = pname.substr(1, pname.size() - 2);
  }
};

template <>
struct FilterAction<prop_name_regex>
{
  template <typename Input>
  static void apply(const Input& in, LimitingClause& clause)
  {
    auto pname = in.string(); // This has single quotes around it.
    // std::cout << "prop_name <" << pname << "> regex\n";
    clause.m_propName = pname.substr(1, pname.size() - 2);
    clause.m_propNameIsRegex = true;
  }
};

template <>
struct FilterAction<prop_int_value>
{
  template <typename Input>
  static void apply(const Input& in, LimitingClause& clause)
  {
    clause.m_propIntValues.push_back(std::stoi(in.string()));
  }
};

template <>
struct FilterAction<prop_fp_value>
{
  template <typename Input>
  static void apply(const Input& in, LimitingClause& clause)
  {
    clause.m_propFloatValues.push_back(std::stod(in.string()));
  }
};

template <>
struct FilterAction<prop_string_value>
{
  template <typename Input>
  static void apply(const Input& in, LimitingClause& clause)
  {
    auto pval = in.string(); // This has single quotes around it. Remove them below.
    clause.m_propStringValues.push_back(pval.substr(1, pval.size() - 2));
    clause.m_propStringIsRegex.push_back(false);
  }
};

template <>
struct FilterAction<prop_string_regex>
{
  template <typename Input>
  static void apply(const Input& in, LimitingClause& clause)
  {
    auto pval = in.string(); // This has single quotes around it. Remove them below.
    clause.m_propStringValues.push_back(pval.substr(1, pval.size() - 2));
    clause.m_propStringIsRegex.push_back(true);
  }
};

} // namespace model
} // namespace smtk

#endif // __smtk_model_FilterGrammar_h
