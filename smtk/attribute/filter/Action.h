//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_filter_Action_h
#define smtk_attribute_filter_Action_h

#include "smtk/resource/filter/Property.h"
#include "smtk/resource/filter/Rules.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"

#include "smtk/Regex.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "tao/pegtl.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#include <memory>

namespace smtk
{
namespace attribute
{
namespace filter
{

using namespace tao::pegtl;

/// A base class template for processing PEGTL rules. A specialization of this
/// class template must exist for each PEGTL rule to be processed.
template<typename Rule>
struct Action : nothing<Rule>
{
};

/// PEGTL requires that a template class be declared with a specialization for
/// each PEGTL rule to be processed. SMTK's Property-based grammar uses a common
/// syntax for each property type, making it practical for us to describe generic
/// Actions templated over the  property type. We then link these actions
/// together with their respective rules using inheritance (as is PEGTL's wont).

/// Construct a new filter rule specific to a given type.
template<typename Type, template<typename T> class RuleClass>
struct TypeNameAction
{
  template<typename Input>
  static void apply(const Input&, smtk::resource::filter::Rules& rules)
  {
    rules.emplace_back(new RuleClass<Type>());
  }
};

/// Append the filter rule with a means of discriminating property keys to match
/// the rule input.
template<typename Type, template<typename T> class RuleClass>
struct NameAction
{
  template<typename Input>
  static void apply(const Input& input, smtk::resource::filter::Rules& rules)
  {
    std::unique_ptr<smtk::resource::filter::Rule>& rule = rules.data().back();
    std::string name = input.string();
    static_cast<RuleClass<Type>*>(rule.get())->acceptableKeys =
      [name](const smtk::resource::PersistentObject& object) -> std::vector<std::string> {
      std::vector<std::string> returnValue;
      if (object.properties().contains<Type>(name))
      {
        returnValue.push_back(name);
      }
      else
      {
        // if the object is an attribute we need to check its definitions, or
        // if the object is definition, we need to check the definitions it is based on
        // If the object is an attribute then check its Definitions
        smtk::attribute::DefinitionPtr def;

        const auto* att = dynamic_cast<const smtk::attribute::Attribute*>(&object);
        if (att)
        {
          def = att->definition();
        }
        else
        {
          const auto* attDef = dynamic_cast<const smtk::attribute::Definition*>(&object);
          if (attDef)
          {
            def = attDef->baseDefinition();
          }
        }
        if (def)
        {
          for (; def != nullptr; def = def->baseDefinition())
          {
            if (def->properties().contains<Type>(name))
            {
              returnValue.push_back(name);
              break;
            }
          }
        }
      }
      return returnValue;
    };
  }
};

/// Append the filter rule with a means of discriminating property keys to match
/// the rule input.
template<typename Type, template<typename T> class RuleClass>
struct RegexAction
{
  template<typename Input>
  static void apply(const Input& input, smtk::resource::filter::Rules& rules)
  {
    std::unique_ptr<smtk::resource::filter::Rule>& rule = rules.data().back();
    smtk::regex regex(input.string());
    static_cast<RuleClass<Type>*>(rule.get())->acceptableKeys =
      [regex](const smtk::resource::PersistentObject& object) -> std::vector<std::string> {
      std::vector<std::string> returnValue;
      for (const auto& key : object.properties().get<Type>().keys())
      {
        if (smtk::regex_match(key, regex))
        {
          returnValue.push_back(key);
        }
      }
      // if the object is an attribute we need to check its definitions, or
      // if the object is definition, we need to check the definitions it is based on
      // If the object is an attribute then check its Definitions
      smtk::attribute::DefinitionPtr def;

      const auto* att = dynamic_cast<const smtk::attribute::Attribute*>(&object);
      if (att)
      {
        def = att->definition();
      }
      else
      {
        const auto* attDef = dynamic_cast<const smtk::attribute::Definition*>(&object);
        if (attDef)
        {
          def = attDef->baseDefinition();
        }
      }
      if (def)
      {
        for (; def != nullptr; def = def->baseDefinition())
        {
          for (const auto& key : def->properties().get<Type>().keys())
          {
            if (smtk::regex_match(key, regex))
            {
              returnValue.push_back(key);
            }
          }
        }
      }
      return returnValue;
    };
  }
};

/// Append the filter rule with a means of discriminating property values to
/// match the rule input.
template<typename Type, template<typename T> class RuleClass>
struct ValueAction
{
  template<typename Input>
  static void apply(const Input& input, smtk::resource::filter::Rules& rules)
  {
    std::unique_ptr<smtk::resource::filter::Rule>& rule = rules.data().back();
    Type value = smtk::resource::filter::Property<Type>::convert(input.string());
    static_cast<RuleClass<Type>*>(rule.get())->acceptableValue = [value](const Type& val) -> bool {
      return val == value;
    };
  }
};

/// Append the filter rule with a means of discriminating property values to
/// match the rule input.
template<typename Type, template<typename T> class RuleClass>
struct ValueRegexAction
{
  template<typename Input>
  static void apply(const Input& input, smtk::resource::filter::Rules& rules)
  {
    std::unique_ptr<smtk::resource::filter::Rule>& rule = rules.data().back();
    smtk::regex regex(input.string());
    static_cast<RuleClass<Type>*>(rule.get())->acceptableValue = [regex](const Type& val) -> bool {
      return smtk::regex_match(val, regex);
    };
  }
};
} // namespace filter
} // namespace attribute
} // namespace smtk

#endif
