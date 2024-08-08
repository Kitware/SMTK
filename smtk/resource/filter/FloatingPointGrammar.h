//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_filter_FloatingPointGrammar_h
#define smtk_resource_filter_FloatingPointGrammar_h

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
/// floating point properties.
template <>
struct Property<double>
{
  struct detail
  {
   /// This fragment has been adapted from PEGTL's example double:
   /// Copyright (c) 2014-2018 Dr. Colin Hirsch and Daniel Frey
   /// Visit https://github.com/taocpp/PEGTL/ for license information.

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
    struct exponent : seq< plus_minus, plus< tao::pegtl::digit > > {};
    struct decimal : seq< number< tao::pegtl::digit >, opt< e, exponent > > {};
    struct hexadecimal : seq< one< '0' >, one< 'x', 'X' >, number< xdigit >,
                              opt< p, exponent > > {};
    struct value : seq< plus_minus, sor< hexadecimal, decimal, inf, nan > > {};
  };

  /// The PEGTL pattern to match when identifying this property type.
  struct TypeNameDescription
    : pad<sor<TAO_PEGTL_ISTRING("double"),
              seq<TAO_PEGTL_ISTRING("float"), opt<TAO_PEGTL_ISTRING("ing-point")> > >, space> {};

  /// The actual type that invokes an action. This must be different from the
  /// pattern definition, so composing grammar rules can refer to the latter
  /// without triggering actions intended for the former.
  struct TypeName : TypeNameDescription {};

  /// Syntax for the property name.
  struct Name : filter::Name<double> {};

  /// Syntax for the property name regex.
  struct Regex : filter::Regex<double> {};

  /// The grammar for this type can refer to property names either by the
  /// explicit name or by a regex that matches a name.
  struct NameRepresentation : sor<pad<quoted<Name>, space>, pad<slashed<Regex>, space> > {};

  /// The PEGTL pattern to match when extracting the property value.
  struct ValueDescription : detail::value {};

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
  static double convert(const std::string& input) { return std::stod(input); }
};

// clang-format on
} // namespace filter
} // namespace resource
} // namespace smtk

#endif
