//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_filter_FloatingPointActions_h
#define smtk_attribute_filter_FloatingPointActions_h

#include "smtk/attribute/filter/Action.h"
#include "smtk/attribute/filter/RuleFor.h"

#include "smtk/resource/filter/Name.h"
#include "smtk/resource/filter/Property.h"
#include "smtk/resource/filter/VectorActions.h"

namespace smtk
{
namespace attribute
{
namespace filter
{

using namespace tao::pegtl;

// clang-format off

/// Actions related to parsing rules for double properties w/r the Attribute Resource
template <> struct Action<smtk::resource::filter::Property<double>::TypeName> : TypeNameAction<double, RuleFor> {};
template <> struct Action<smtk::resource::filter::Property<double>::Name> : NameAction<double, RuleFor> {};
template <> struct Action<smtk::resource::filter::Property<double>::Regex> : RegexAction<double, RuleFor> {};
template <> struct Action<smtk::resource::filter::Property<double>::Value> : ValueAction<double, RuleFor> {};

/// Actions related to parsing rules for vectors of this type.
template <> struct Action<smtk::resource::filter::Property<std::vector<double> >::TypeName> :
    TypeNameAction<std::vector<double>, RuleFor > {};
template <> struct Action<smtk::resource::filter::Property<std::vector<double> >::Name> :
    NameAction<std::vector<double>, RuleFor > {};
template <> struct Action<smtk::resource::filter::Property<std::vector<double> >::Regex> :
    RegexAction<std::vector<double>, RuleFor > {};
template <> struct Action<smtk::resource::filter::Property<std::vector<double> >::Value> :
    smtk::resource::filter::ValueAction<std::vector<double>, RuleFor > {};
// clang-format on
} // namespace filter
} // namespace attribute
} // namespace smtk

#endif
