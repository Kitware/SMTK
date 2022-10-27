//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_model_FilterGrammar_h
#define smtk_model_FilterGrammar_h
/*!\file FilterGrammar.h - PEGTL structures for parsing resource filter strings */

#include "smtk/model/LimitingClause.h"

#include "tao/pegtl.hpp"

namespace smtk
{
namespace model
{

using namespace tao::pegtl;

// clang-format off
/// @cond dox_ignore
// -----
/// Describe a grammar for floating point values.
///
/// This fragment has been adapted from PEGTL's example double:
/// Copyright (c) 2014-2018 Dr. Colin Hirsch and Daniel Frey
/// Visit https://github.com/taocpp/PEGTL/ for license information.
namespace floating_point
{
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
struct value : seq< plus_minus, sor< hexadecimal, decimal, inf, nan > > {};
}
// -----

/// Define a syntax for expressions that occur inside a pair of symbols, and
/// construct named specializations that fit our grammar.
template <char left, char right, typename... arguments>
struct enclosed : if_must<one<left>, until<one<right>,  arguments...> > {};

template <typename... arguments>
struct bracketed : enclosed<'[', ']', arguments...> {};

template <typename... arguments>
struct braced : enclosed<'{', '}', arguments...> {};

template <typename... arguments>
struct parenthesized : enclosed<'(', ')', arguments...> {};

template <typename... arguments>
struct quoted : enclosed<'\'', '\'', arguments...> {};

template <typename... arguments>
struct slashed : enclosed<'/', '/', arguments...> {};

// Define tags for the three supported property types.
struct int_property_name : pad<TAO_PEGTL_ISTRING("integer"), space> {};
struct float_property_name : pad<TAO_PEGTL_ISTRING("floating-point"), space> {};
struct string_property_name : pad<TAO_PEGTL_ISTRING("string"), space> {};

/// Define a syntax for property names, regex strings, and values.
///
/// NB: - Enclosed values (e.g., quoted and slashed values) accept any ASCII
///       symbol other than the enclosing symbols.
///     - There are two structs for each property element in the query: the first
///       matches the value we wish to extract, and the second matches the
///       pattern of the requested element. We embed the former into the latter
///       to facilitate the retrieval of the value without modification.

struct name_property_value : plus<not_one<'\''> > {};
struct name_property : pad<quoted<name_property_value>, space> {};

struct name_property_regex_value : plus<not_one<'/'> > {};
struct name_property_regex : pad<slashed<name_property_regex_value>, space> {};

struct int_property_value : plus<digit> {};
struct int_property : pad<int_property_value, space> {};

struct float_property_value : floating_point::value {};
struct float_property : pad<float_property_value, space>{};

struct string_property_value : plus<not_one<'\''> > {};
struct string_property : pad<quoted<string_property_value>, space> {};

struct string_property_regex_value : plus<not_one<'/'> > {};
struct string_property_regex : pad<slashed<string_property_regex_value>, space> {};

/// All property types may be a list of values or a single value. To accommodate
/// this pattern, we describe a property sequence as a list of values.
template <typename property>
struct property_sequence
  : pad<sor<property, parenthesized<list<property, one<','>, space> > >, space> {};

/// @endcond
// clang-format on

/// We use a traits class to describe analogous features between the different
/// property types.
template<typename property>
struct property_traits;

template<>
struct property_traits<int_property>
{
  typedef int_property_name name;
  typedef property_sequence<int_property> sequence;
};

template<>
struct property_traits<float_property>
{
  typedef float_property_name name;
  typedef property_sequence<float_property> sequence;
};

template<>
struct property_traits<string_property>
{
  typedef string_property_name name;
  typedef property_sequence<sor<string_property, string_property_regex>> sequence;
};

/// With the differences between the property types factored out into the above
/// traits class, we can now construct a general description for the grammar for
/// each property type.
template<typename property>
struct grammar_for
  : pad<
      seq<
        typename property_traits<property>::name,
        opt<braced<
          sor<name_property, name_property_regex>,
          opt<pad<TAO_PEGTL_ISTRING("="), space>, typename property_traits<property>::sequence>>>>,
      space>
{
};

/// The filter grammar is a composition of the grammar for each property type.
struct SMTKCORE_EXPORT FilterGrammar
  : bracketed<
      sor<grammar_for<int_property>, grammar_for<float_property>, grammar_for<string_property>>>
{
};

/// Actions on the state in response to encountered grammar.
template<typename Rule>
struct SMTKCORE_EXPORT FilterAction : nothing<Rule>
{
};

template<>
struct FilterAction<int_property_name>
{
  template<typename Input>
  static void apply(const Input&, LimitingClause& clause)
  {
    clause.m_propType = smtk::resource::PropertyType::INTEGER_PROPERTY;
  }
};

template<>
struct FilterAction<float_property_name>
{
  template<typename Input>
  static void apply(const Input&, LimitingClause& clause)
  {
    clause.m_propType = smtk::resource::PropertyType::FLOAT_PROPERTY;
  }
};

template<>
struct FilterAction<string_property_name>
{
  template<typename Input>
  static void apply(const Input&, LimitingClause& clause)
  {
    clause.m_propType = smtk::resource::PropertyType::STRING_PROPERTY;
  }
};

template<>
struct FilterAction<name_property_value>
{
  template<typename Input>
  static void apply(const Input& in, LimitingClause& clause)
  {
    clause.m_propName = in.string();
  }
};

template<>
struct FilterAction<name_property_regex_value>
{
  template<typename Input>
  static void apply(const Input& in, LimitingClause& clause)
  {
    clause.m_propName = in.string();
    clause.m_propNameIsRegex = true;
  }
};

template<>
struct FilterAction<int_property_value>
{
  template<typename Input>
  static void apply(const Input& in, LimitingClause& clause)
  {
    clause.m_propIntValues.push_back(std::stoi(in.string()));
  }
};

template<>
struct FilterAction<float_property_value>
{
  template<typename Input>
  static void apply(const Input& in, LimitingClause& clause)
  {
    clause.m_propFloatValues.push_back(std::stod(in.string()));
  }
};

template<>
struct FilterAction<string_property_value>
{
  template<typename Input>
  static void apply(const Input& in, LimitingClause& clause)
  {
    clause.m_propStringValues.push_back(in.string());
    clause.m_propStringIsRegex.push_back(false);
  }
};

template<>
struct FilterAction<string_property_regex_value>
{
  template<typename Input>
  static void apply(const Input& in, LimitingClause& clause)
  {
    clause.m_propStringValues.push_back(in.string());
    clause.m_propStringIsRegex.push_back(true);
  }
};
} // namespace model
} // namespace smtk

#endif // smtk_model_FilterGrammar_h
