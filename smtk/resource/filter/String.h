//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_filter_String_h
#define smtk_resource_filter_String_h

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

/// Actions related to parsing rules for this type.
template <> struct Action<Property<std::string>::TypeName> : TypeNameAction<std::string> {};
template <> struct Action<Property<std::string>::Name> : NameAction<std::string> {};
template <> struct Action<Property<std::string>::Regex> : RegexAction<std::string> {};
template <> struct Action<Property<std::string>::Value> : ValueAction<std::string> {};
template <> struct Action<Property<std::string>::ValueRegex> : ValueRegexAction<std::string> {};

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

/// Actions related to parsing rules for this type.
template <> struct Action<Property<std::vector<std::string> >::TypeName>
  : TypeNameAction<std::vector<std::string> > {};
template <> struct Action<Property<std::vector<std::string> >::Name>
  : NameAction<std::vector<std::string> > {};
template <> struct Action<Property<std::vector<std::string> >::Regex>
  : RegexAction<std::vector<std::string> > {};
// clang-format on

/// Specialization of ValueAction to accommodate vectors of types.
template<>
struct Action<Property<std::vector<std::string>>::Value>
{
  template<typename Input>
  static void apply(const Input& input, Rules& rules)
  {
    std::unique_ptr<Rule>& rule = rules.data().back();
    std::vector<std::string> value;
    smtk::regex re(",");
    std::string in = input.string();
    std::sregex_token_iterator it(in.begin(), in.end(), re, -1), last;
    for (int id = 0; it != last; ++it, ++id)
    {
      std::string str = it->str();
      str = str.substr(1 + str.find_first_of('\''));
      str = str.substr(0, str.size() - 1);
      value.push_back(str);
    }

    static_cast<RuleFor<std::vector<std::string>>*>(rule.get())->acceptableValue =
      [value](const std::vector<std::string>& val) -> bool {
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
template<>
struct Action<Property<std::vector<std::string>>::ValueRegex>
{
  template<typename Input>
  static void apply(const Input& input, Rules& rules)
  {
    std::unique_ptr<Rule>& rule = rules.data().back();
    std::vector<smtk::regex> regex;

    smtk::regex re(",");
    std::sregex_token_iterator it(input.string().begin(), input.string().end(), re, -1), last;
    for (int id = 0; it != last; ++it, ++id)
    {
      std::string str = it->str();
      str = str.substr(1 + str.find_first_of('/'));
      str = str.substr(0, str.size() - 1);
      regex.emplace_back(str.c_str());
    }

    static_cast<RuleFor<std::vector<std::string>>*>(rule.get())->acceptableValue =
      [regex](const std::vector<std::string>& val) -> bool {
      if (val.size() != regex.size())
      {
        return false;
      }
      for (std::size_t i = 0; i < regex.size(); ++i)
      {
        if (!smtk::regex_match(val[i], regex[i]))
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
