//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_filter_Enclosed_h
#define smtk_resource_filter_Enclosed_h

#include "tao/pegtl.hpp"

namespace smtk
{
namespace resource
{
namespace filter
{

using namespace tao::pegtl;

// clang-format off
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
// clang-format on
} // namespace filter
} // namespace resource
} // namespace smtk

#endif
