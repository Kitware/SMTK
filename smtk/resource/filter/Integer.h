//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_filter_Integer_h
#define smtk_resource_filter_Integer_h

#include "smtk/resource/filter/Name.h"
#include "smtk/resource/filter/Property.h"

namespace smtk
{
namespace resource
{
namespace filter
{

using namespace tao::pegtl;

// clang-format off

/// A specialization for describing the grammar for parsing rules related to
/// integer properties.
template <>
struct Property<long>
{
  /// The PEGTL pattern to match when identifying this property type.
  struct TypeNameDescription
    : pad<sor<TAO_PEGTL_ISTRING("long"),
              seq<TAO_PEGTL_ISTRING("int"), opt<TAO_PEGTL_ISTRING("eger")> > >, space> {};

  /// The actual type that invokes an action. This must be different from the
  /// pattern definition, so composing grammar rules can refer to the latter
  /// without triggering actions intended for the former.
  struct TypeName : TypeNameDescription {};

  /// Syntax for the property name.
  struct Name : filter::Name<long> {};

  /// Syntax for the property name regex.
  struct Regex : filter::Regex<long> {};

  /// The grammar for this type can refer to property names either by the
  /// explicit name or by a regex that matches a name.
  struct NameRepresentation : sor<pad<quoted<Name>, space>, pad<slashed<Regex>, space> > {};

  /// The PEGTL pattern to match when extracting the property value.
  struct ValueDescription : plus<tao::pegtl::digit> {};

  /// The actual type that invokes an action. This must be different from the
  /// pattern definition, so composing grammar rules can refer to the latter
  /// without triggering actions intended for the former.
  struct Value : ValueDescription {};

  /// The PETL pattern to match when identifying this property value.
  struct ValueRepresentation : pad<Value, space> {};

  /// The grammar for this type is a composition of the above elements.
  struct Grammar : pad<seq<TypeName,
                           opt<braced<NameRepresentation,
                                      opt<pad<TAO_PEGTL_ISTRING("="), space>, ValueRepresentation> > > >,
      space> {};

  /// Convert a string into a value of this type.
  static long convert(const std::string& input) { return std::stol(input); }
};

/// Actions related to parsing rules for this type.
template <> struct Action<Property<long>::TypeName> : TypeNameAction<long> {};
template <> struct Action<Property<long>::Name> : NameAction<long> {};
template <> struct Action<Property<long>::Regex> : RegexAction<long> {};
template <> struct Action<Property<long>::Value> : ValueAction<long> {};

/// Actions related to parsing rules for vectors of this type.
template <> struct Action<Property<std::vector<long> >::TypeName> :
    TypeNameAction<std::vector<long> > {};
template <> struct Action<Property<std::vector<long> >::Name> :
    NameAction<std::vector<long> > {};
template <> struct Action<Property<std::vector<long> >::Regex> :
    RegexAction<std::vector<long> > {};
template <> struct Action<Property<std::vector<long> >::Value> :
    ValueAction<std::vector<long> > {};
// clang-format on
} // namespace filter
} // namespace resource
} // namespace smtk

#endif
