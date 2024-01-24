//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_graph_arcs_DeleteArcs_h
#define smtk_graph_arcs_DeleteArcs_h

#include "smtk/graph/ArcImplementation.h"
#include "smtk/graph/ArcProperties.h"

#include <algorithm>
#include <array>
#include <memory>

namespace smtk
{
namespace graph
{
namespace evaluators
{

/**\brief This is a functor that deletes arcs terminating at a node.
  */
struct SMTKCORE_EXPORT DeleteArcs
{
  DeleteArcs() = default;

  static void begin(...) {}
  static void end(...) {}

  // For arcs that are bidirectionally indexed, we can handle incoming arcs easily.
  template<typename Impl, typename ArcTraits = typename Impl::Traits, typename Resource>
  typename std::enable_if<!ArcProperties<ArcTraits>::isOnlyForwardIndexed::value, void>::type
  operator()(const Impl* arcs, const Resource*, Component* node, bool explicitOnly, bool& didRemove)
    const
  {
    (void)arcs;
    if (!ArcProperties<Impl>::isExplicit::value && explicitOnly)
    {
      return;
    }
    if (auto from = dynamic_cast<typename ArcTraits::FromType*>(node))
    {
      didRemove |= from->template outgoing<ArcTraits>().disconnect(nullptr);
    }
    else if (auto to = dynamic_cast<typename ArcTraits::ToType*>(node))
    {
      didRemove |= to->template incoming<ArcTraits>().disconnect(nullptr);
    }
  }

  // For arcs that are forward-indexed only, we have to visit the entire map to
  // remove arcs whose destination matches \a node.
  template<typename Impl, typename ArcTraits = typename Impl::Traits, typename Resource>
  typename std::enable_if<ArcProperties<ArcTraits>::isOnlyForwardIndexed::value, void>::type
  operator()(const Impl* arcs, const Resource*, Component* node, bool explicitOnly, bool& didRemove)
    const
  {
    (void)arcs;
    if (!ArcProperties<Impl>::isExplicit::value && explicitOnly)
    {
      return;
    }
    if (auto from = dynamic_cast<typename ArcTraits::FromType*>(node))
    {
      didRemove |= from->template outgoing<ArcTraits>().disconnect(nullptr);
    }
    else if (auto to = dynamic_cast<typename ArcTraits::ToType*>(node))
    {
      // TODO: Iterate
      // didRemove |= to->template incoming<ArcTraits>().disconnect(nullptr);
    }
  }

  // For run-time arcs, we only support bidirectionally indexed variants for the moment.
  template<typename Resource>
  void operator()(
    smtk::string::Token arcTypeName,
    ArcImplementationBase& arcs,
    const Resource*,
    Component* node,
    bool explicitOnly,
    bool& didRemove) const
  {
    (void)arcs;
    // Runtime arcs are always explicit.
    (void)explicitOnly;

    didRemove |= node->outgoing(arcTypeName).disconnect(nullptr);
    didRemove |= node->incoming(arcTypeName).disconnect(nullptr);
  }
};

} // namespace evaluators
} // namespace graph
} // namespace smtk

#endif // smtk_graph_arcs_DeleteArcs_h
