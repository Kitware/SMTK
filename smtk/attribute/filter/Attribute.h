//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_filter_Attribute_h
#define smtk_attribute_filter_Attribute_h

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/resource/filter/Action.h"
#include "smtk/resource/filter/Enclosed.h"
#include "smtk/resource/filter/Name.h"
#include "smtk/resource/filter/Property.h"
#include "smtk/resource/filter/Rules.h"

#include <regex>

namespace smtk
{
namespace attribute
{
namespace filter
{

using namespace tao::pegtl;

/// Description for grammar describing an Attribute's Definition Type specification.
/// The following are valid examples:
///  * type = 'foo' : Would match an Attribute whose Definition derivation contains a Definition with type() = foo
///  * type= /base.*/ : Would match an Attribute whose Definition derivation contains a Definition with type() that would matches the regex "base.*""
struct AttributeTypeSpec
{
  /// Syntax for the Attribute's Definition type name.
  struct TypeName : plus<not_one<'\''>>
  {
  };

  /// Syntax for the Attribute's Definition type name regex.
  struct TypeRegex : plus<not_one<'/'>>
  {
  };

  /// The grammar for this type is a composition of the above elements.
  struct Grammar
    : pad<
        seq<
          TAO_PEGTL_ISTRING("type"),
          star<space>,
          TAO_PEGTL_ISTRING("="),
          star<space>,
          sor<
            pad<smtk::resource::filter::quoted<TypeName>, space>,
            pad<smtk::resource::filter::slashed<TypeRegex>, space>>>,
        space>
  {
  };
};

// Rule for dealing with type name matching for an Attribute
class TypeNameRule : public smtk::resource::filter::Rule
{
public:
  TypeNameRule(const std::string& typeName)
    : m_typeName(typeName)
  {
  }

  ~TypeNameRule() override = default;

  bool operator()(const smtk::resource::PersistentObject& object) const override
  {
    const auto* attribute = dynamic_cast<const smtk::attribute::Attribute*>(&object);
    if (attribute)
    {
      // Find the target Definition
      auto targetDef = attribute->attributeResource()->findDefinition(m_typeName);
      // Is the attribute derived from that Definition?
      return (targetDef && attribute->isA(targetDef));
    }
    return false;
  }
  std::string m_typeName;
};

// Rule for dealing with type regex matching for an Attribute
class TypeRegexRule : public smtk::resource::filter::Rule
{
public:
  TypeRegexRule(const std::string& typeRegex)
    : m_typeRegex(typeRegex)
  {
  }

  ~TypeRegexRule() override = default;

  bool operator()(const smtk::resource::PersistentObject& object) const override
  {
    // Generate the Regex based on the string
    std::regex regex(m_typeRegex);
    const auto* attribute = dynamic_cast<const smtk::attribute::Attribute*>(&object);
    if (attribute)
    {
      // Walk the Attribute's derivation to see if any Definition matches the regex pattern
      for (smtk::attribute::DefinitionPtr def = attribute->definition(); def != nullptr;
           def = def->baseDefinition())
      {
        if (std::regex_match(def->type(), regex))
        {
          return true;
        }
      }
    }
    return false;
  }
  std::string m_typeRegex;
};

/// Actions related to parsing rules for this type. They basically insert the
/// appropriate rule into the set of rules.
struct TypeNameAction
{
  template<typename Input>
  static void apply(const Input& input, smtk::resource::filter::Rules& rules)
  {
    std::string typeName = input.string();
    rules.emplace_back(new TypeNameRule(typeName));
  }
};

struct TypeRegexAction
{
  template<typename Input>
  static void apply(const Input& input, smtk::resource::filter::Rules& rules)
  {
    std::string typeRegex = input.string();
    rules.emplace_back(new TypeRegexRule(typeRegex));
  }
};

} // namespace filter
} // namespace attribute
/// Since the Action template class is defined in the resource::filter name space, we need be in that
/// namespace to do template specialization.
namespace resource
{
namespace filter
{
template<>
struct Action<smtk::attribute::filter::AttributeTypeSpec::TypeName>
  : smtk::attribute::filter::TypeNameAction
{
};
template<>
struct Action<smtk::attribute::filter::AttributeTypeSpec::TypeRegex>
  : smtk::attribute::filter::TypeRegexAction
{
};
} // namespace filter
} // namespace resource
} // namespace smtk

#endif
