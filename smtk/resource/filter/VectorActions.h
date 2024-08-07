//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_filter_VectorActions_h
#define smtk_resource_filter_VectorActions_h

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

/// Specialization of ValueAction to accommodate vectors of types.
template<typename Type, template<typename T> class RuleClass>
struct ValueAction<std::vector<Type>, RuleClass>
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

    static_cast<RuleClass<std::vector<Type>>*>(rule.get())->acceptableValue =
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
template<typename Type, template<typename T> class RuleClass>
struct ValueRegexAction<std::vector<Type>, RuleClass>
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

    static_cast<RuleClass<std::vector<Type>>*>(rule.get())->acceptableValue =
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
