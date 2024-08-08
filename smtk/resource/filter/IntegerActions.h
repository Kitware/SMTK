//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_filter_IntegerActions_h
#define smtk_resource_filter_IntegerActions_h

#include "smtk/resource/filter/Action.h"
#include "smtk/resource/filter/IntegerGrammar.h"
#include "smtk/resource/filter/Name.h"
#include "smtk/resource/filter/RuleFor.h"
#include "smtk/resource/filter/VectorActions.h"

namespace smtk
{
namespace resource
{
namespace filter
{

using namespace tao::pegtl;

// clang-format off

/// Actions related to parsing rules for integers
template <> struct Action<Property<long>::TypeName> : TypeNameAction<long, RuleFor> {};
template <> struct Action<Property<long>::Name> : NameAction<long, RuleFor> {};
template <> struct Action<Property<long>::Regex> : RegexAction<long, RuleFor> {};
template <> struct Action<Property<long>::Value> : ValueAction<long, RuleFor> {};

/// Actions related to parsing rules for vectors of this type.
template <> struct Action<Property<std::vector<long>>::TypeName> :
    TypeNameAction<std::vector<long>, RuleFor  > {};
template <> struct Action<Property<std::vector<long> >::Name> :
    NameAction<std::vector<long>, RuleFor > {};
template <> struct Action<Property<std::vector<long> >::Regex> :
    RegexAction<std::vector<long>, RuleFor > {};
template <> struct Action<Property<std::vector<long> >::Value> :
    ValueAction<std::vector<long>, RuleFor > {};
// clang-format on
} // namespace filter
} // namespace resource
} // namespace smtk

#endif
