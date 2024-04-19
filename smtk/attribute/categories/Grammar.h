//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_categories_Grammar_h
#define smtk_attribute_categories_Grammar_h

#include "tao/pegtl.hpp"

using namespace tao::pegtl;

// clang-format off

namespace smtk
{
namespace attribute
{
namespace categories
{

/// Define a syntax for expressions that occur inside a pair of symbols, and
/// construct named specializations that fit our grammar.
template <char left, char right, typename... arguments>
struct enclosed : if_must<one<left>, until<one<right>,  arguments...> > {};

template <typename... arguments>
struct parenthesized : enclosed<'(', ')', arguments...> {};

template <typename... arguments>
struct quoted : enclosed<'\'', '\'', arguments...> {};

struct ExpressionSyntax;

struct BracketSyntax
   : tao::pegtl::if_must< one< '(' >, ExpressionSyntax, tao::pegtl::one< ')' > > {};

struct NameSyntax : plus<not_one<'\''>>
{
};

struct BareNameSyntax : identifier
{
};

struct SMTKCORE_EXPORT CategoryNameSyntax
  : sor<
      quoted<NameSyntax>, BareNameSyntax>
{
};

struct SMTKCORE_EXPORT ComplementOperator
  : TAO_PEGTL_ISTRING("!")
{
};
struct SMTKCORE_EXPORT ComplementSyntax
  : pad<seq<
      ComplementOperator, ExpressionSyntax>, space>
{
};

struct SMTKCORE_EXPORT OperandSyntax
  : sor<CategoryNameSyntax, BracketSyntax, ComplementSyntax>
{
};

struct SMTKCORE_EXPORT AndOperator
  : TAO_PEGTL_ISTRING("&")
{
};
struct SMTKCORE_EXPORT OrOperator
  : TAO_PEGTL_ISTRING("|")
{
};
struct SMTKCORE_EXPORT BinaryOperator
  : sor<AndOperator, OrOperator>
{
};

struct SMTKCORE_EXPORT ExpressionSyntax
    : list<OperandSyntax, BinaryOperator, space>
{
};

struct SMTKCORE_EXPORT ExpressionGrammar
  : must<
    ExpressionSyntax, eof>
{
};

// clang-format on

} // namespace categories
} // namespace attribute
} // namespace smtk

#endif
