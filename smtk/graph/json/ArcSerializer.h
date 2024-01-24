//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_graph_json_ArcSerializer_h
#define smtk_graph_json_ArcSerializer_h

#include "smtk/graph/ArcImplementation.h"
#include "smtk/graph/ArcProperties.h"

#include "nlohmann/json.hpp"

namespace smtk
{
namespace graph
{
using json = nlohmann::json;

struct ArcSerializer
{
  template<typename ResourceType>
  static void begin(const ResourceType*, json&)
  {
  }

  // Handle compile-time arc types.
  template<typename Impl, typename ArcTraits = typename Impl::Traits, typename ResourceType>
  void operator()(const Impl* arcs, const ResourceType* resource, json& jj) const
  {
    if (!arcs || !arcs->isExplicit())
    {
      return;
    }
    json jArcs;
    arcs->visitAllOutgoingNodes(resource, [&jArcs](const typename ArcTraits::FromType* node) {
      json jDestinations = json::array();
      node->template outgoing<ArcTraits>().visit(
        [&jDestinations](const typename ArcTraits::ToType* other) {
          jDestinations.push_back(other->id());
        });
      if (!jDestinations.empty())
      {
        jArcs[node->id().toString()] = jDestinations;
      }
    });
    if (!jArcs.empty())
    {
      jj[smtk::common::typeName<ArcTraits>()] = jArcs;
    }
  }

  // Handle run-time arc types.
  template<typename ResourceType>
  void operator()(
    smtk::string::Token arcTypeName,
    const ArcImplementationBase& arcs,
    const ResourceType* resource,
    json& jj) const
  {
    // For now, we assume all run-time arcs are explicit and mutable.

    json jArcs;
    arcs.visitOutgoingNodes(
      resource, arcTypeName, [&jArcs, &arcTypeName](const smtk::graph::Component* node) {
        json jDestinations = json::array();
        node->outgoing(arcTypeName).visit([&jDestinations](const smtk::graph::Component* other) {
          jDestinations.push_back(other->id());
          return smtk::common::Visit::Continue;
        });
        if (!jDestinations.empty())
        {
          jArcs[node->id().toString()] = jDestinations;
        }
        return smtk::common::Visit::Continue;
      });
    if (!jArcs.empty())
    {
      jj[arcTypeName.data()] = jArcs;
    }
  }

  template<typename ResourceType>
  static void end(const ResourceType*, json&)
  {
  }
};

} // namespace graph
} // namespace smtk

#endif // smtk_graph_json_ArcSerializer_h
