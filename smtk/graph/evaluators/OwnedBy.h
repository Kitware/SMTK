//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_graph_evaluators_OwnedNodes_h
#define smtk_graph_evaluators_OwnedNodes_h

#include "smtk/graph/ArcImplementation.h"
#include "smtk/graph/ArcProperties.h"

#include "smtk/string/Token.h"

#include <algorithm>
#include <array>
#include <memory>
#include <set>
#include <sstream>

namespace smtk
{
namespace graph
{
namespace evaluators
{

/**\brief This is a functor that accumulates nodes which "own" nodes in an input set.
  *
  * For arcs that have relevant ownership semantics, this functor will visit nodes
  * in the input set and insert nodes connected to them into the output set.
  */
struct SMTKCORE_EXPORT OwnedNodes
{
  OwnedNodes() {}

  static void begin(...) {}
  static void end(...) {}

  // Support compile-time arcs.
  template<typename Impl, typename ArcTraits = typename Impl::Traits, typename ResourcePtr>
  void operator()(
    const Impl* arcs,
    ResourcePtr resource,
    const std::set<smtk::graph::Component*>& inputs,
    std::set<smtk::graph::Component*>& outputs,
    bool& addedDependencies) const
  {
    (void)arcs;
    (void)resource;
    std::string arcType = smtk::common::typeName<ArcTraits>();
    smtk::string::Token arcToken(arcType);
    auto semantics = ArcProperties<ArcTraits>::ownershipSemantics();
    if (semantics == OwnershipSemantics::None)
    {
      return;
    }
    if (semantics == OwnershipSemantics::FromNodeOwnsToNode)
    {
      for (const auto& input : inputs)
      {
        if (auto* node = dynamic_cast<typename ArcTraits::FromType*>(input))
        {
          node->template outgoing<ArcTraits>().visit(
            [&outputs, &inputs, &addedDependencies](const typename ArcTraits::ToType* to) {
              auto* mto = const_cast<typename ArcTraits::ToType*>(to);
              outputs.insert(mto);
              if (inputs.find(mto) == inputs.end())
              {
                addedDependencies = true;
              }
            });
        }
      }
    }
    else if (semantics == OwnershipSemantics::ToNodeOwnsFromNode)
    {
      for (const auto& input : inputs)
      {
        if (auto* node = dynamic_cast<typename ArcTraits::ToType*>(input))
        {
          node->template incoming<ArcTraits>().visit(
            [&outputs, &inputs, &addedDependencies](const typename ArcTraits::FromType* from) {
              auto* mfrom = const_cast<typename ArcTraits::FromType*>(from);
              outputs.insert(mfrom);
              if (inputs.find(mfrom) == inputs.end())
              {
                addedDependencies = true;
              }
            });
        }
      }
    }
  }

  // Do not support run-time arcs.
  template<typename ResourcePtr>
  void operator()(
    smtk::string::Token arcTypeName,
    const ArcImplementationBase& arcs,
    ResourcePtr resource,
    const std::set<smtk::graph::Component*>& inputs,
    std::set<smtk::graph::Component*>& outputs,
    bool& addedDependencies) const
  {
    (void)arcTypeName;
    (void)arcs;
    (void)resource;
    (void)inputs;
    (void)outputs;
    (void)addedDependencies;
  }
};

} // namespace evaluators
} // namespace graph
} // namespace smtk

#endif // smtk_graph_evaluators_OwnedNodes_h
