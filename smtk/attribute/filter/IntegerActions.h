//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_filter_IntegerActions_h
#define smtk_attribute_filter_IntegerActions_h

#include "smtk/attribute/filter/Action.h"
#include "smtk/attribute/filter/RuleFor.h"
#include "smtk/resource/filter/IntegerGrammar.h"
#include "smtk/resource/filter/Name.h"
#include "smtk/resource/filter/VectorActions.h"

namespace smtk
{
namespace attribute
{
namespace filter
{

using namespace tao::pegtl;

// clang-format off

/// Actions related to parsing rules for integer properties w/r the Attribute Resource
template <> struct Action<smtk::resource::filter::Property<long>::TypeName> : TypeNameAction<long, RuleFor> {};
template <> struct Action<smtk::resource::filter::Property<long>::Name> : NameAction<long, RuleFor> {};
template <> struct Action<smtk::resource::filter::Property<long>::Regex> : RegexAction<long, RuleFor> {};
template <> struct Action<smtk::resource::filter::Property<long>::Value> : ValueAction<long, RuleFor> {};

/// Actions related to parsing rules for vectors of this type.
template <> struct Action<smtk::resource::filter::Property<std::vector<long>>::TypeName> :
    TypeNameAction<std::vector<long>, RuleFor  > {};
template <> struct Action<smtk::resource::filter::Property<std::vector<long> >::Name> :
    NameAction<std::vector<long>, RuleFor > {};
template <> struct Action<smtk::resource::filter::Property<std::vector<long> >::Regex> :
    RegexAction<std::vector<long>, RuleFor > {};
template <> struct Action<smtk::resource::filter::Property<std::vector<long> >::Value> :
    smtk::resource::filter::ValueAction<std::vector<long>, RuleFor > {};
// clang-format on
} // namespace filter
} // namespace attribute
} // namespace smtk

#endif
