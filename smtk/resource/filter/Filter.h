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
#include "smtk/resource/filter/Rules.h"

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
template<typename GrammarType = Grammar>
class Filter
{
public:
  Filter(const std::string& str)
    : m_filterString(str)
    , m_rules(std::move(constructRules(str)))
  {
  }
  virtual ~Filter() = default;

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
    , m_rules(std::move(constructRules(other.m_filterString)))
  {
  }

  Filter(Filter&& other) noexcept
    : m_filterString(std::move(other.m_filterString))
    , m_rules(std::move(other.m_rules))
  {
  }

  Filter& operator=(const Filter& other)
  {
    m_filterString = other.m_filterString;
    m_rules = constructRules(other.m_filterString);
    return *this;
  }

  Filter& operator=(Filter&& other) noexcept
  {
    m_filterString = other.m_filterString;
    m_rules = std::move(other.m_rules);
    return *this;
  }

  bool operator()(const Component& component) const { return m_rules(component); }

private:
  smtk::resource::filter::Rules constructRules(const std::string& filterString)
  {
    smtk::resource::filter::Rules rules;

    tao::pegtl::string_input<> in(filterString, "constructRule");
    try
    {
      tao::pegtl::parse<GrammarType, smtk::resource::filter::Action>(in, rules);
    }
    catch (tao::pegtl::parse_error& err)
    {
      const auto p = err.positions.front();
#if TAO_PEGTL_VERSION_MAJOR <= 2 && TAO_PEGTL_VERSION_MINOR <= 7
      smtkErrorMacro(
        smtk::io::Logger::instance(),
        "smtk::resource::filter::Filter: " << err.what() << "\n"
                                           << in.line_as_string(p) << "\n"
                                           << std::string(p.byte_in_line, ' ') << "^\n");
#else
      smtkErrorMacro(
        smtk::io::Logger::instance(),
        "smtk::resource::filter::Filter: " << err.what() << "\n"
                                           << in.line_at(p) << "\n"
                                           << std::string(p.byte_in_line, ' ') << "^\n");
#endif
    }

    return rules;
  }

  std::string m_filterString;
  smtk::resource::filter::Rules m_rules;
};
} // namespace filter
} // namespace resource
} // namespace smtk

#endif
