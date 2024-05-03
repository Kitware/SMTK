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
#include "smtk/attribute/filter/Action.h"
#include "smtk/attribute/filter/Grammar.h"
#include "smtk/resource/filter/Enclosed.h"
#include "smtk/resource/filter/Name.h"
#include "smtk/resource/filter/Property.h"
#include "smtk/resource/filter/Rules.h"

#include "smtk/Regex.h"

namespace smtk
{
namespace attribute
{
namespace filter
{

using namespace tao::pegtl;

struct ComponentHeader
  : sor<
      TAO_PEGTL_ISTRING("attribute"),
      TAO_PEGTL_ISTRING("definition"),
      TAO_PEGTL_ISTRING("smtk::attribute::Attribute"),
      TAO_PEGTL_ISTRING("smtk::attribute::Definition"),
      TAO_PEGTL_STRING("*"),
      TAO_PEGTL_ISTRING("any")>
{
};

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
    smtk::regex regex(m_typeRegex);
    const auto* attribute = dynamic_cast<const smtk::attribute::Attribute*>(&object);
    if (attribute)
    {
      // Walk the Attribute's derivation to see if any Definition matches the regex pattern
      for (smtk::attribute::DefinitionPtr def = attribute->definition(); def != nullptr;
           def = def->baseDefinition())
      {
        if (smtk::regex_match(def->type(), regex))
        {
          return true;
        }
      }
    }
    return false;
  }
  std::string m_typeRegex;
};

// Rule for dealing with component header information
class ComponentHeaderRule : public smtk::resource::filter::Rule
{
public:
  ComponentHeaderRule(const std::string& componentType)
    : m_componentType(componentType)
  {
  }

  ~ComponentHeaderRule() override = default;

  bool operator()(const smtk::resource::PersistentObject& object) const override
  {
    // Allow "any" and "*" to always pass
    if ((m_componentType == "any") || (m_componentType == "*"))
    {
      return true;
    }
    if ((m_componentType == "attribute") || (m_componentType == "smtk::attribute::Attribute"))
    {
      const auto* attribute = dynamic_cast<const smtk::attribute::Attribute*>(&object);
      return (attribute != nullptr);
    }
    if ((m_componentType == "definition") || (m_componentType == "smtk::attribute::Definition"))
    {
      const auto* definition = dynamic_cast<const smtk::attribute::Definition*>(&object);
      return (definition != nullptr);
    }
    // Unsupported Component Type
    return false;
  }
  std::string m_componentType;
};

/// Actions related to parsing rules for this type. They basically insert the
/// appropriate rule into the set of rules.
struct ComponentHeaderAction
{
  template<typename Input>
  static void apply(const Input& input, smtk::resource::filter::Rules& rules)
  {
    std::string componentTypeName = input.string();
    rules.emplace_back(new ComponentHeaderRule(componentTypeName));
  }
};

struct AttributeTypeSpecTypeNameAction
{
  template<typename Input>
  static void apply(const Input& input, smtk::resource::filter::Rules& rules)
  {
    std::string typeName = input.string();
    rules.emplace_back(new TypeNameRule(typeName));
  }
};

struct AttributeTypeSpecTypeRegexAction
{
  template<typename Input>
  static void apply(const Input& input, smtk::resource::filter::Rules& rules)
  {
    std::string typeRegex = input.string();
    rules.emplace_back(new TypeRegexRule(typeRegex));
  }
};

template<>
struct Action<ComponentHeader> : ComponentHeaderAction
{
};
template<>
struct Action<AttributeTypeSpec::TypeName> : AttributeTypeSpecTypeNameAction
{
};
template<>
struct Action<AttributeTypeSpec::TypeRegex> : AttributeTypeSpecTypeRegexAction
{
};
} // namespace filter
} // namespace attribute
} // namespace smtk

#endif
