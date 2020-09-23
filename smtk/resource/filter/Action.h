//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_filter_Action_h
#define smtk_resource_filter_Action_h

#include "smtk/resource/filter/Property.h"
#include "smtk/resource/filter/Rule.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "tao/pegtl.hpp"
// PEGTL does not itself appear to do anything nasty, but
// on Windows MSVC 2015, it includes something that defines
// a macro named ERROR to be 0. This causes smtkErrorMacro()
// to expand into garbage (because smtk::io::Logger::ERROR
// gets expanded to smtk::io::Logger::0).
#ifdef ERROR
#undef ERROR
#endif
SMTK_THIRDPARTY_POST_INCLUDE

#include <memory>
#include <regex>

namespace smtk
{
namespace resource
{
namespace filter
{

using namespace tao::pegtl;

/// A base class template for processing PEGTL rules. A specialization of this
/// class template must exist for each PEGTL rule to be processed.
template <typename Rule>
struct Action : nothing<Rule>
{
};

/// PEGTL requires that a template class be declared with a specialization for
/// each PEGTL rule to be processed. SMTK's Property-based grammar uses a common
/// syntax for each property type, making it practical for us to describe generic
/// Actions templated over the  property type. We then link these actions
/// together with their respective rules using inheritance (as is PEGTL's wont).

/// Construct a new filter rule specific to a given type.
template <typename Type>
struct TypeNameAction
{
  template <typename Input>
  static void apply(const Input&, std::unique_ptr<Rule>& functor)
  {
    functor.reset(new RuleFor<Type>());
  }
};

/// Append the filter rule with a means of discriminating property keys to match
/// the rule input.
template <typename Type>
struct NameAction
{
  template <typename Input>
  static void apply(const Input& input, std::unique_ptr<Rule>& functor)
  {
    std::string name = input.string();
    static_cast<RuleFor<Type>*>(functor.get())->acceptableKeys = [name](
      const PersistentObject& object) -> std::vector<std::string> {
      std::vector<std::string> returnValue;
      if (object.properties().contains<Type>(name))
      {
        returnValue.push_back(name);
      }
      return returnValue;
    };
  }
};

/// Append the filter rule with a means of discriminating property keys to match
/// the rule input.
template <typename Type>
struct RegexAction
{
  template <typename Input>
  static void apply(const Input& input, std::unique_ptr<Rule>& functor)
  {
    std::regex regex(input.string());
    static_cast<RuleFor<Type>*>(functor.get())->acceptableKeys = [regex](
      const PersistentObject& object) -> std::vector<std::string> {
      std::vector<std::string> returnValue;
      for (const auto& key : object.properties().get<Type>().keys())
      {
        if (std::regex_match(key, regex))
        {
          returnValue.push_back(key);
        }
      }
      return returnValue;
    };
  }
};

/// Append the filter rule with a means of discriminating property values to
/// match the rule input.
template <typename Type>
struct ValueAction
{
  template <typename Input>
  static void apply(const Input& input, std::unique_ptr<Rule>& functor)
  {
    Type value = Property<Type>::convert(input.string());
    static_cast<RuleFor<Type>*>(functor.get())->acceptableValue = [value](
      const Type& val) -> bool { return val == value; };
  }
};

/// Append the filter rule with a means of discriminating property values to
/// match the rule input.
template <typename Type>
struct ValueRegexAction
{
  template <typename Input>
  static void apply(const Input& input, std::unique_ptr<Rule>& functor)
  {
    std::regex regex(input.string());
    static_cast<RuleFor<Type>*>(functor.get())->acceptableValue = [regex](
      const Type& val) -> bool { return std::regex_match(val, regex); };
  }
};
}
}
}

#endif
