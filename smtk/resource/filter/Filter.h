//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_filter_Filter_h
#define smtk_resource_filter_Filter_h

#include "smtk/CoreExports.h"

#include "smtk/io/Logger.h"

#include "smtk/resource/Component.h"

#include "smtk/resource/filter/Action.h"
#include "smtk/resource/filter/Grammar.h"
#include "smtk/resource/filter/Rule.h"

#include <string>

namespace smtk
{
namespace resource
{
namespace filter
{

/// A functor for filtering components using PEGTL.
///
/// Given a PEGTL grammar, smtk::resource::filter::Filter is a copyable functor
/// that converts a filter string into a set of filter rules.
template <typename GrammarType = Grammar>
class Filter
{
public:
  Filter(const std::string& str)
    : m_filterString(str)
    , m_rule(constructRule(str))
  {
  }

  // Specific filter rules are composed by parsing string inputs, and are
  // therefore inherently runtime-constructed objects (and, thus, are allocated
  // on the heap). To accommodate their memory management, these objects are
  // stored using a std::unique_ptr, which has move-only semantics.
  // smtk:::resource::filter::Filter must satisfy the API for
  // smtk::resource::Resource::queryOperation, which returns a std::function by
  // value. As a result, this class Defines copy construction and assignment by
  // reconstructing the internal rule when necessary.

  Filter(const Filter& other)
    : m_filterString(other.m_filterString)
    , m_rule(constructRule(other.m_filterString))
  {
  }

  Filter(Filter&& other)
    : m_filterString(other.m_filterString)
    , m_rule(std::move(other.m_rule))
  {
  }

  Filter& operator=(const Filter& other)
  {
    m_filterString = other.m_filterString;
    m_rule = constructRule(other.m_filterString);
    return *this;
  }

  Filter& operator=(Filter&& other)
  {
    m_filterString = other.m_filterString;
    m_rule = std::move(other.m_rule);
    return *this;
  }

  bool operator()(const Component& component) const
  {
    if (m_rule)
    {
      return (*m_rule)(component);
    }
    return false;
  }

private:
  static std::unique_ptr<smtk::resource::filter::Rule> constructRule(
    const std::string& filterString)
  {
    std::unique_ptr<smtk::resource::filter::Rule> rule;

    tao::pegtl::string_input<> in(filterString, "constructRule");
    try
    {
      tao::pegtl::parse<GrammarType, smtk::resource::filter::Action>(in, rule);
    }
    catch (tao::pegtl::parse_error& err)
    {
      const auto p = err.positions.front();
      smtkErrorMacro(smtk::io::Logger::instance(),
        "smtk::resource::filter::Filter: " << err.what() << "\n"
                                           << in.line_as_string(p) << "\n"
                                           << std::string(p.byte_in_line, ' ') << "^\n");
      rule.release();
    }

    return rule;
  }

  std::string m_filterString;
  std::unique_ptr<smtk::resource::filter::Rule> m_rule;
};
}
}
}

#endif
