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
  template<typename ResourcePtr>
  static void begin(ResourcePtr, json&)
  {
  }

  template<typename Impl, typename ArcTraits = typename Impl::Traits, typename ResourcePtr>
  void operator()(const Impl* arcs, ResourcePtr resource, json& jj) const
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

  template<typename ResourcePtr>
  static void end(ResourcePtr, json&)
  {
  }
};

} // namespace graph
} // namespace smtk

#endif // smtk_graph_json_ArcSerializer_h
