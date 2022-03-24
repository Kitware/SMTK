//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_filter_Vector_h
#define smtk_resource_filter_Vector_h

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

/// Specialization of ValueAction to accommodate vectors of types.
template<typename Type>
struct ValueAction<std::vector<Type>>
{
  template<typename Input>
  static void apply(const Input& input, Rules& rules)
  {
    std::unique_ptr<Rule>& rule = rules.data().back();
    std::vector<Type> value;
    std::regex re(",");
    std::string in = input.string();
    std::sregex_token_iterator it(in.begin(), in.end(), re, -1), last;
    for (int id = 0; it != last; ++it, ++id)
    {
      std::string str = it->str();
      value.push_back(Property<Type>::convert(str));
    }

    static_cast<RuleFor<std::vector<Type>>*>(rule.get())->acceptableValue =
      [value](const std::vector<Type>& val) -> bool {
      if (val.size() != value.size())
      {
        return false;
      }
      for (std::size_t i = 0; i < value.size(); ++i)
      {
        if (val[i] != value[i])
        {
          return false;
        }
      }
      return true;
    };
  }
};

/// Specialization of ValueRegexAction to accommodate vectors of types.
template<typename Type>
struct ValueRegexAction<std::vector<Type>>
{
  template<typename Input>
  static void apply(const Input& input, Rules& rules)
  {
    std::unique_ptr<Rule>& rule = rules.data().back();
    std::vector<std::regex> regex;

    std::regex re(",");
    std::sregex_token_iterator it(input.string().begin(), input.string().end(), re, -1), last;
    for (int id = 0; it != last; ++it, ++id)
    {
      regex.emplace_back(it->str().c_str());
    }

    static_cast<RuleFor<std::vector<Type>>*>(rule.get())->acceptableValue =
      [regex](const std::vector<Type>& val) -> bool {
      if (val.size() != regex.size())
      {
        return false;
      }
      for (std::size_t i = 0; i < regex.size(); ++i)
      {
        if (!std::regex_match(val[i], regex[i]))
        {
          return false;
        }
      }
      return true;
    };
  }
};
} // namespace filter
} // namespace resource
} // namespace smtk

#endif
