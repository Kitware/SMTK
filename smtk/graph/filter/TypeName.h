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

#include <regex>

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
  /// Syntax for the property name.
  struct Name : smtk::resource::filter::Name<smtk::graph::Component> {};

  /// Syntax for the property name regex.
  struct Regex : smtk::resource::filter::Regex<smtk::graph::Component> {};

  /// The grammar for this type can refer to property names either by the
  /// explicit name or by a regex that matches a name.
  struct Representation : sor<pad<smtk::resource::filter::quoted<Name>, space>,
                              pad<smtk::resource::filter::slashed<Regex>, space> > {};

  /// The grammar for this type is a composition of the above elements.
  struct Grammar : pad<Representation, space> {};

  class Rule : public smtk::resource::filter::Rule
  {
  public:
    ~Rule() override = default;

    bool operator()(const smtk::resource::PersistentObject& object) const override
    {
      return (object.typeName() == value);
    }

    std::string value;
  };

  class RegexRule : public smtk::resource::filter::Rule
  {
  public:
    ~RegexRule() override = default;

    bool operator()(const smtk::resource::PersistentObject& object) const override
    {
      return std::regex_match(object.typeName(), std::regex(value));
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
struct Action<smtk::graph::filter::TypeName::Name>
{
  template<typename Input>
  static void apply(const Input& input, Rules& rules)
  {
    rules.emplace_back(new smtk::graph::filter::TypeName::Rule());
    std::unique_ptr<Rule>& rule = rules.data().back();
    static_cast<smtk::graph::filter::TypeName::Rule*>(rule.get())->value = input.string();
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
} // namespace filter
} // namespace resource
} // namespace smtk

#endif
