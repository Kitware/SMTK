//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_filter_VectorGrammar_h
#define smtk_resource_filter_VectorGrammar_h

#include "smtk/resource/filter/Name.h"
#include "smtk/resource/filter/Property.h"

#include <algorithm>
#include <cctype>

namespace smtk
{
namespace resource
{
namespace filter
{

using namespace tao::pegtl;

// clang-format off

/// A specialization for describing the grammar for parsing rules related to
/// vector properties.
template <typename T>
struct Property<std::vector<T> >
{
  /// The PEGTL pattern to match when identifying this property type.
  struct TypeNameDescription : pad<seq<TAO_PEGTL_ISTRING("vector<"),
                                       typename Property<T>::TypeNameDescription,
                                       TAO_PEGTL_ISTRING(">")>, space> {};

  /// The actual type that invokes an action. This must be different from the
  /// pattern definition, so composing grammar rules can refer to the latter
  /// without triggering actions intended for the former.
  struct TypeName : TypeNameDescription {};

  /// Syntax for the property name.
  struct Name : filter::Name<std::vector<T> > {};

  /// Syntax for the property name regex.
  struct Regex : filter::Regex<std::vector<T> > {};

  /// The grammar for this type can refer to property names either by the
  /// explicit name or by a regex that matches a name.
  struct NameRepresentation : sor<pad<quoted<Name>, space>, pad<slashed<Regex>, space> > {};

  /// The PEGTL pattern to match when extracting the property value.
  struct Value : list<typename Property<T>::ValueDescription, one<','>, space> {};

  /// The PETL pattern to match when identifying this property value.
  struct ValueRepresentation : pad<parenthesized<pad<Value, space> >, space> {};

  /// The grammar for this type is a composition of the above elements.
  struct Grammar : pad<seq<TypeName,
                           opt<braced<NameRepresentation,
                                      opt<pad<TAO_PEGTL_ISTRING("="), space>, ValueRepresentation> > > >,
      space> {};
};
// clang-format on

} // namespace filter
} // namespace resource
} // namespace smtk

#endif
