//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_graph_filter_TypeName_h
#define smtk_graph_filter_TypeName_h

#include "smtk/resource/filter/Action.h"
#include "smtk/resource/filter/Name.h"
#include "smtk/resource/filter/Rule.h"

#include "smtk/common/StringUtil.h"

#include "smtk/string/Token.h"

#include "smtk/Regex.h"

namespace smtk
{
namespace graph
{
namespace filter
{

using namespace tao::pegtl;

// clang-format off

struct TypeName
{
  /// Syntax for a component type-name.
  struct Name : smtk::resource::filter::Name<smtk::graph::Component> {};

  /// Syntax for a bare component type-name.
  struct BareTypeName : smtk::resource::filter::BareName<smtk::graph::Component> {};

  /// Syntax for the property name regex.
  struct Regex : smtk::resource::filter::Regex<smtk::graph::Component> {};

  /// Accept an empty filter (containing zero or more spaces) to indicate only resources,
  /// not components, should match. If you use this in a "sor<>" grammar, it must be last
  /// so it is only matched when all else fails.
  struct EmptyNameIsResource : star<space> {};

  /// Accept "any" or "*" as special component types that match any typename.
  struct AnyOrStar : sor<TAO_PEGTL_ISTRING("*"),TAO_PEGTL_ISTRING("any")> {};

  /// The grammar for this type can refer to property names either by the
  /// explicit name or by a regex that matches a name.
  struct Representation : sor<pad<AnyOrStar, space>,
                              pad<smtk::resource::filter::quoted<Name>, space>,
                              pad<smtk::resource::filter::slashed<Regex>, space>,
                              pad<BareTypeName, space>,
                              EmptyNameIsResource> {};

  /// The grammar for this type is a composition of the above elements.
  struct Grammar : pad<Representation, space> {};

  /// A rule that only accepts resources (not components).
  class OnlyMatchResource : public smtk::resource::filter::Rule
  {
  public:
    ~OnlyMatchResource() override = default;

    bool operator()(const smtk::resource::PersistentObject& object) const override
    {
      return !!dynamic_cast<const smtk::graph::ResourceBase*>(&object);
    }

  };

  /// A rule that only accepts exact type-name matches.
  class Exact : public smtk::resource::filter::Rule
  {
  public:
    ~Exact() override = default;

    bool operator()(const smtk::resource::PersistentObject& object) const override
    {
      return (object.typeName() == value);
    }

    std::string value;
  };

  /// A rule that accepts exact type-name matches as well as subclass matches.
  class ExactPlusInherited : public smtk::resource::filter::Rule
  {
  public:
    ~ExactPlusInherited() override = default;

    bool operator()(const smtk::resource::PersistentObject& object) const override
    {
      return object.matchesType(value);
    }

    smtk::string::Token value;
  };

  /// A rule that accepts regex type-name matches.
  class RegexRule : public smtk::resource::filter::Rule
  {
  public:
    ~RegexRule() override = default;

    bool operator()(const smtk::resource::PersistentObject& object) const override
    {
      return smtk::regex_match(object.typeName(), smtk::regex(value));
    }

    std::string value;
  };
};
}
}
}
// clang-format on

namespace smtk
{
namespace resource
{
namespace filter
{
/// Actions related to parsing rules for this type.
template<>
struct Action<smtk::graph::filter::TypeName::EmptyNameIsResource>
{
  template<typename Input>
  static void apply(const Input& input, Rules& rules)
  {
    (void)input;
    rules.emplace_back(new smtk::graph::filter::TypeName::OnlyMatchResource());
  }
};

template<>
struct Action<smtk::graph::filter::TypeName::Name>
{
  template<typename Input>
  static void apply(const Input& input, Rules& rules)
  {
    rules.emplace_back(new smtk::graph::filter::TypeName::Exact());
    std::unique_ptr<Rule>& rule = rules.data().back();
    static_cast<smtk::graph::filter::TypeName::Exact*>(rule.get())->value = input.string();
  }
};

template<>
struct Action<smtk::graph::filter::TypeName::Regex>
{
  template<typename Input>
  static void apply(const Input& input, Rules& rules)
  {
    rules.emplace_back(new smtk::graph::filter::TypeName::RegexRule());
    std::unique_ptr<Rule>& rule = rules.data().back();
    static_cast<smtk::graph::filter::TypeName::RegexRule*>(rule.get())->value = input.string();
  }
};

template<>
struct Action<smtk::graph::filter::TypeName::BareTypeName>
{
  template<typename Input>
  static void apply(const Input& input, Rules& rules)
  {
    rules.emplace_back(new smtk::graph::filter::TypeName::ExactPlusInherited());
    std::unique_ptr<Rule>& rule = rules.data().back();
    std::string matchName = input.string();
    static_cast<smtk::graph::filter::TypeName::ExactPlusInherited*>(rule.get())->value =
      smtk::common::StringUtil::trim(matchName);
  }
};
} // namespace filter
} // namespace resource
} // namespace smtk

#endif
