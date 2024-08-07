//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_filter_FloatingPointActions_h
#define smtk_resource_filter_FloatingPointActions_h

#include "smtk/resource/filter/Action.h"
#include "smtk/resource/filter/FloatingPointGrammar.h"
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

/// Actions related to parsing rules for doubles.
template <> struct Action<Property<double>::TypeName> : TypeNameAction<double, RuleFor> {};
template <> struct Action<Property<double>::Name> : NameAction<double, RuleFor> {};
template <> struct Action<Property<double>::Regex> : RegexAction<double, RuleFor> {};
template <> struct Action<Property<double>::Value> : ValueAction<double, RuleFor> {};

/// Actions related to parsing rules for vectors of this type.
template <> struct Action<Property<std::vector<double> >::TypeName> :
    TypeNameAction<std::vector<double>, RuleFor > {};
template <> struct Action<Property<std::vector<double> >::Name> :
    NameAction<std::vector<double>, RuleFor > {};
template <> struct Action<Property<std::vector<double> >::Regex> :
    RegexAction<std::vector<double>, RuleFor > {};
template <> struct Action<Property<std::vector<double> >::Value> :
    ValueAction<std::vector<double>, RuleFor > {};
// clang-format on
} // namespace filter
} // namespace resource
} // namespace smtk

#endif
