//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_graph_evaluators_OwnersOf_h
#define smtk_graph_evaluators_OwnersOf_h

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

/**\brief This functor accumulates nodes that own its \a inputs.
  *
  * For arcs that have relevant ownership semantics, this functor will visit nodes
  * in the input set and insert connected nodes into the \a outputs set.
  *
  * ```cpp
  *   smtk::graph::Component* A;
  *   smtk::graph::Component* B;
  *   resource->evaluateArcs<OwnersOf>({A, B}, outputs, addedDependencies);
  * ```
  *
  * Note that \a outputs may overlap \a input, since if A and B are in the input
  * and A owns B, A will also appear in the output.
  * However – while A will appear in \a outputs – \a addedDependencies will be
  * false as long as no other nodes are inserted into \a outputs:
  * \a addedDependencies indicates whether \a outputs is a subset of \a inputs.
  */
struct SMTKCORE_EXPORT OwnersOf
{
  OwnersOf() = default;

  template<typename ResourceType>
  static void begin(
    const ResourceType*,
    const std::set<smtk::graph::Component*>&,
    std::set<smtk::graph::Component*>&,
    bool&)
  {
  }

  template<typename ResourceType>
  static void end(
    const ResourceType*,
    const std::set<smtk::graph::Component*>&,
    std::set<smtk::graph::Component*>&,
    bool&)
  {
  }

  // Support compile-time arcs.
  template<typename Impl, typename ArcTraits = typename Impl::Traits, typename ResourceType>
  void operator()(
    const Impl* arcs,
    const ResourceType* resource,
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
        if (auto* node = dynamic_cast<typename ArcTraits::ToType*>(input))
        {
          node->template incoming<ArcTraits>().visit(
            [&outputs, &inputs, &addedDependencies](const typename ArcTraits::FromType* from) {
              auto* mutFrom = const_cast<typename ArcTraits::FromType*>(from);
              if (outputs.insert(mutFrom).second)
              {
                if (inputs.find(mutFrom) == inputs.end())
                {
                  addedDependencies = true;
                }
              }
            });
        }
      }
    }
    else if (semantics == OwnershipSemantics::ToNodeOwnsFromNode)
    {
      for (const auto& input : inputs)
      {
        if (auto* node = dynamic_cast<typename ArcTraits::FromType*>(input))
        {
          node->template outgoing<ArcTraits>().visit(
            [&outputs, &inputs, &addedDependencies](const typename ArcTraits::ToType* to) {
              auto* mutTo = const_cast<typename ArcTraits::ToType*>(to);
              if (outputs.insert(mutTo).second)
              {
                if (inputs.find(mutTo) == inputs.end())
                {
                  addedDependencies = true;
                }
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

#endif // smtk_graph_evaluators_OwnersOf_h
