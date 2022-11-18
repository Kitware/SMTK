//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_graph_json_ArcDeserializer_h
#define smtk_graph_json_ArcDeserializer_h

#include "smtk/graph/ArcImplementation.h"
#include "smtk/graph/ArcProperties.h"

#include "smtk/io/Logger.h"

#include "nlohmann/json.hpp"

namespace smtk
{
namespace graph
{
using json = nlohmann::json;

struct ArcDeserializer
{
  template<typename ResourceType>
  static void begin(ResourceType*, const json&)
  {
  }

  template<typename ResourceType, typename Impl, bool Mutable>
  struct TrueDeserializer
  {
    void operator()(Impl* arcs, ResourceType* resource, const json& jj) const
    {
      (void)arcs;
      (void)resource;
      (void)jj;
      // Do nothing.
    }
  };

  template<typename ResourceType, typename Impl>
  struct TrueDeserializer<ResourceType, Impl, true>
  {
    void operator()(Impl* arcs, ResourceType* resource, const json& jj) const
    {
      if (!arcs)
      {
        return;
      }

      std::string arcType = smtk::common::typeName<typename Impl::Traits>();
      auto it = jj.find(arcType);
      if (it == jj.end())
      {
        return;
      }

      // Iterate over all the objects
      for (const auto& entry : it->items())
      {
        smtk::common::UUID fromId(entry.key());
        auto fromNode =
          dynamic_cast<const typename Impl::Traits::FromType*>(resource->component(fromId));
        if (!fromNode)
        {
          smtkErrorMacro(
            smtk::io::Logger::instance(),
            "Asked to create an arc from non-existent node " << fromId << ".");
          continue;
        }
        for (const auto& jTo : entry.value().items())
        {
          auto toId = jTo.value().get<smtk::common::UUID>();
          auto toNode =
            dynamic_cast<const typename Impl::Traits::ToType*>(resource->component(toId));
          if (!toNode)
          {
            smtkErrorMacro(
              smtk::io::Logger::instance(),
              "Asked to create an arc to non-existent node " << toId << ".");
            continue;
          }
          // TODO: For ordered arcs, we can easily handle the "beforeTo"
          //       argument, but "beforeFrom" is impossible to determine
          //       from what is currently stored.
          arcs->connect(fromNode, toNode);
        }
      }
    }
  };

  template<typename ResourceType, typename Impl>
  void operator()(Impl* arcs, ResourceType* resource, const json& jj) const
  {
    // Dispatch to different handlers based on mutability of the arc type.
    TrueDeserializer<ResourceType, Impl, ArcProperties<typename Impl::Traits>::isMutable::value>()(
      arcs, resource, jj);
  }

  template<typename ResourceType>
  static void end(const ResourceType*, const json&)
  {
  }
};

} // namespace graph
} // namespace smtk

#endif // smtk_graph_json_ArcDeserializer_h
