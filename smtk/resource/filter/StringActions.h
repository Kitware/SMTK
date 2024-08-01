//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_filter_StringActions_h
#define smtk_resource_filter_StringActions_h

#include "smtk/resource/filter/Action.h"
#include "smtk/resource/filter/Name.h"
#include "smtk/resource/filter/RuleFor.h"
#include "smtk/resource/filter/StringGrammar.h"
#include "smtk/resource/filter/VectorActions.h"

#include "smtk/Regex.h"

namespace smtk
{
namespace resource
{
namespace filter
{

using namespace tao::pegtl;

// clang-format off

/// Actions related to parsing rules for strings.
template <> struct Action<Property<std::string>::TypeName> : TypeNameAction<std::string, RuleFor> {};
template <> struct Action<Property<std::string>::Name> : NameAction<std::string, RuleFor> {};
template <> struct Action<Property<std::string>::Regex> : RegexAction<std::string, RuleFor> {};
template <> struct Action<Property<std::string>::Value> : ValueAction<std::string, RuleFor> {};
template <> struct Action<Property<std::string>::ValueRegex> : ValueRegexAction<std::string, RuleFor> {};


/// Actions related to parsing rules for this type.
template <> struct Action<Property<std::vector<std::string> >::TypeName>
  : TypeNameAction<std::vector<std::string>, RuleFor > {};
template <> struct Action<Property<std::vector<std::string> >::Name>
  : NameAction<std::vector<std::string>, RuleFor > {};
template <> struct Action<Property<std::vector<std::string> >::Regex>
  : RegexAction<std::vector<std::string>, RuleFor > {};
// clang-format on

/// Specialization of ValueAction to accommodate vectors of strings.
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

/// Specialization of ValueRegexAction to accommodate vectors of strings.
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
