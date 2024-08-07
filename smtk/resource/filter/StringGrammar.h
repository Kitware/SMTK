//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_filter_StringGrammar_h
#define smtk_resource_filter_StringGrammar_h

#include "smtk/resource/filter/Name.h"
#include "smtk/resource/filter/Property.h"

#include "smtk/Regex.h"

namespace smtk
{
namespace resource
{
namespace filter
{

using namespace tao::pegtl;

// clang-format off

/// A specialization for describing the grammar for parsing rules related to
/// string properties.
template <>
struct Property<std::string>
{
  /// The PEGTL pattern to match when identifying this property type.
  struct TypeNameDescription : pad<TAO_PEGTL_ISTRING("string"), space> {};

  /// The actual type that invokes an action. This must be different from the
  /// pattern definition, so composing grammar rules can refer to the latter
  /// without triggering actions intended for the former.
  struct TypeName : TypeNameDescription {};

  /// Syntax for the property name.
  struct Name : filter::Name<std::string> {};

  /// Syntax for the property name regex.
  struct Regex : filter::Regex<std::string> {};

  /// The grammar for this type can refer to property names either by the
  /// explicit name or by a regex that matches a name.
  struct NameRepresentation : sor<pad<quoted<Name>, space>, pad<slashed<Regex>, space> > {};

  /// The PEGTL pattern to match when extracting the property value.
  struct ValueDescription : plus<not_one<'\''> > {};

  /// The actual type that invokes an action. This must be different from the
  /// pattern definition, so composing grammar rules can refer to the latter
  /// without triggering actions intended for the former.
  struct Value : ValueDescription {};

  /// The PETL pattern to match when identifying this property value.
  struct ValueRepresentation : pad<quoted<Value>, space> {};

  /// The PEGTL pattern to match when extracting the property value via regex.
  struct ValueRegexDescription : plus<not_one<'/'> > {};

  /// The actual type that invokes an action. This must be different from the
  /// pattern definition, so composing grammar rules can refer to the latter
  /// without triggering actions intended for the former.
  struct ValueRegex : ValueRegexDescription {};

  /// The PETL pattern to match when identifying this property value regex.
  struct ValueRegexRepresentation : pad<slashed<ValueRegex>, space> {};

  /// The grammar for this type is a composition of the above elements.
  struct Grammar
    : pad<seq<TypeName,
              opt<braced<NameRepresentation,
                         opt<pad<TAO_PEGTL_ISTRING("="), space>,
                             sor<ValueRepresentation, ValueRegexRepresentation> > > > >,
          space> {};

  /// Convert a string into a value of this type.
  static const std::string& convert(const std::string& input) { return input; }
};

/// A specialization for describing the grammar for parsing rules related to
/// vector<string> properties. This specialization is explicit to accommodate
/// referencing values via regex.
template <>
struct Property<std::vector<std::string> >
{
  /// The PEGTL pattern to match when identifying this property type.
  struct TypeNameDescription : pad<seq<TAO_PEGTL_ISTRING("vector<"),
                                       typename Property<std::string>::TypeNameDescription,
                                       TAO_PEGTL_ISTRING(">")>, space> {};

  /// The actual type that invokes an action. This must be different from the
  /// pattern definition, so composing grammar rules can refer to the latter
  /// without triggering actions intended for the former.
  struct TypeName : TypeNameDescription {};

  /// Syntax for the property name.
  struct Name : filter::Name<std::vector<std::string> > {};

  /// Syntax for the property name regex.
  struct Regex : filter::Regex<std::vector<std::string> > {};

  /// The grammar for this type can refer to property names either by the
  /// explicit name or by a regex that matches a name.
  struct NameRepresentation : sor<pad<quoted<Name>, space>, pad<slashed<Regex>, space> > {};

  /// The PEGTL pattern to match when extracting the property value.
  struct Value : list<quoted<typename Property<std::string>::ValueDescription>, one<','>, space> {};

  /// The PEGTL pattern to match when identifying this property value.
  struct ValueRepresentation : pad<parenthesized<pad<Value, space> >, space> {};

  /// The PEGTL pattern to match when extracting the property value regex.
  struct ValueRegex
    : list<quoted<typename Property<std::string>::ValueRegexDescription>, one<','>, space> {};

  /// The PETL pattern to match when identifying this property value regex.
  struct ValueRegexRepresentation : pad<parenthesized<pad<ValueRegex, space> >, space> {};

  /// The grammar for this type is a composition of the above elements.
  struct Grammar
    : pad<seq<TypeName,
              opt<braced<NameRepresentation,
                         opt<pad<TAO_PEGTL_ISTRING("="), space>,
                             sor<ValueRepresentation, ValueRegexRepresentation> > > > >,
          space> {};
};

// clang-format on

} // namespace filter
} // namespace resource
} // namespace smtk

#endif
