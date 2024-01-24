//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_graph_RuntimeArc_h
#define smtk_graph_RuntimeArc_h

#include "smtk/graph/Component.h"
#include "smtk/graph/Directionality.h"
#include "smtk/string/Token.h"

#include <unordered_set>
#include <vector>

namespace smtk
{
namespace graph
{

/**\brief A base class for arc-types defined at runtime.
  *
  */
template<Directionality Direction>
struct SMTK_ALWAYS_EXPORT RuntimeArc
{
  using FromType = Component;
  using ToType = Component;
  using Directed =
    typename std::conditional<Direction == IsDirected, std::true_type, std::false_type>::type;

  RuntimeArc() = default;
  RuntimeArc(
    smtk::string::Token declaredType,
    const std::unordered_set<smtk::string::Token>& fromNodeSpecs,
    const std::unordered_set<smtk::string::Token>& toNodeSpecs,
    smtk::graph::ResourceBase* resource)
    : m_selfType(declaredType)
    , m_fromNodeSpecs(fromNodeSpecs)
    , m_toNodeSpecs(toNodeSpecs)
  {
    // Pre-parse spec strings into test functions:
    for (const auto& fromNodeSpec : fromNodeSpecs)
    {
      if (!fromNodeSpec.valid())
      {
        continue;
      }
      m_fromNodeOps.emplace_back(resource->queryOperation(fromNodeSpec.data()));
    }
    for (const auto& toNodeSpec : toNodeSpecs)
    {
      if (!toNodeSpec.valid())
      {
        continue;
      }
      m_toNodeOps.emplace_back(resource->queryOperation(toNodeSpec.data()));
    }
  }

  /// Providing this method does not replace the implementation
  /// of ExplicitArcs::connect(); instead, this method is used
  /// as an additional run-time check on whether a connection
  /// is allowed.
  bool connect(const Component* from, const Component* to, const Component*, const Component*)
  {
    if (!from || !to)
    {
      return false;
    }
    // Note the single quote is required by the filter grammar.
    smtk::string::Token fromType = "'" + from->typeName() + "'";
    smtk::string::Token toType = "'" + to->typeName() + "'";
    if (!RuntimeArc<Direction>::checkNodeMatch(from, fromType, m_fromNodeSpecs, m_fromNodeOps))
    {
      return false;
    }
    if (!RuntimeArc<Direction>::checkNodeMatch(to, toType, m_toNodeSpecs, m_toNodeOps))
    {
      return false;
    }
    return true;
  }

  static bool checkNodeMatch(
    const Component* node,
    smtk::string::Token typeName,
    const std::unordered_set<smtk::string::Token>& typeRules,
    const std::vector<std::function<bool(const Component&)>>& opRules)
  {
    // First, see if we exactly match any token. (This is fast.)
    auto it = typeRules.find(typeName);
    if (it != typeRules.end())
    {
      return true;
    }
    // No? Then we must parse each type rule and test for matches.
    // (This is slow.)
    return std::any_of(
      opRules.begin(), opRules.end(), [node](std::function<bool(const Component&)> opRule) {
        return opRule(*node);
      });
  }

  smtk::string::Token m_selfType;
  std::unordered_set<smtk::string::Token> m_fromNodeSpecs;
  std::unordered_set<smtk::string::Token> m_toNodeSpecs;
  std::vector<std::function<bool(const Component&)>> m_fromNodeOps;
  std::vector<std::function<bool(const Component&)>> m_toNodeOps;
};

} // namespace graph
} // namespace smtk

#endif // smtk_graph_RuntimeArc_h
