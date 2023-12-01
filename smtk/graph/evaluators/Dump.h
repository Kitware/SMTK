//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_graph_arcs_Dump_h
#define smtk_graph_arcs_Dump_h

#include "smtk/graph/ArcImplementation.h"
#include "smtk/graph/ArcProperties.h"
#include "smtk/graph/Component.h"

#include "smtk/string/Token.h"

#include "smtk/common/Color.h"

#include "nlohmann/json.hpp"

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
using json = nlohmann::json;

/**\brief This is a functor that prints nodes and arcs in plain-text or graphviz format.
  *
  * The only valid values for \a mimeType are "text/vnd.graphviz" or "text/plain".
  */
struct SMTKCORE_EXPORT Dump
{
  Dump()
    : m_mimeType("text/vnd.graphviz")
  {
  }

  Dump(smtk::string::Token mimeType)
    : m_mimeType(mimeType)
  {
  }

  Dump(smtk::string::Token mimeType, const std::set<smtk::string::Token>& whitelist)
    : m_mimeType(mimeType)
    , m_includeArcs(whitelist)
  {
  }

  Dump(
    smtk::string::Token mimeType,
    const std::set<smtk::string::Token>& whitelist,
    const std::set<smtk::string::Token>& blacklist)
    : m_mimeType(mimeType)
    , m_includeArcs(whitelist)
    , m_excludeArcs(blacklist)
  {
  }

  void setArcColor(const smtk::string::Token& arcType, const std::array<double, 4>& color)
  {
    m_arcColors[arcType] = color;
  }

  static void setBackground(const std::array<double, 4>& bgcolor)
  {
    s_backgroundColor = std::unique_ptr<std::array<double, 4>>(new std::array<double, 4>(bgcolor));
  }

  template<typename ResourceType>
  static void begin(const ResourceType* resource, std::ostream& stream, const Dump& self)
  {
    if (!resource)
    {
      return;
    }
    std::function<void(const std::shared_ptr<smtk::resource::Component>&)> mapNodes =
      [&stream, &self](const smtk::resource::ComponentPtr& comp) {
        int nodeLabel = self.m_nextNodeId++;
        self.m_nodeMap[comp->id()] = nodeLabel;
      };
    if (self.m_mimeType == "text/vnd.graphviz")
    {
      stream << "digraph \"" << resource->name() << "\" {\n\n";
      if (s_backgroundColor)
      {
        stream << "  bgcolor=\""
               << smtk::common::Color::floatRGBAToString(s_backgroundColor->data()) << "\"\n";
      };
      if (self.m_includeNodes)
      {
        std::function<void(const std::shared_ptr<smtk::resource::Component>&)> dumpNodes =
          [&stream, &self](const smtk::resource::ComponentPtr& comp) {
            int nodeLabel = self.m_nextNodeId++;
            self.m_nodeMap[comp->id()] = nodeLabel;
            // To make a valid CSS class name from a namespace-qualified C++ name,
            // replace colons with underscores:
            std::string nodeClass = comp->typeName();
            std::replace(nodeClass.begin(), nodeClass.end(), ':', '_');
            stream << "  " << nodeLabel << " [class=\"" << nodeClass << "\", label=\""
                   << comp->name() << "\"]\n";
          };
        resource->visit(dumpNodes);
        stream << "\n";
      }
      else
      {
        resource->visit(mapNodes);
      }
    }
    else // self.m_mimeType == "text/plain"
    {
      if (self.m_includeNodes)
      {
        stream << "---\nNodes of " << resource->name() << "\n";
        std::function<void(const std::shared_ptr<smtk::resource::Component>&)> dumpNodes =
          [&stream, &self](const smtk::resource::ComponentPtr& comp) {
            int nodeLabel = self.m_nextNodeId++;
            self.m_nodeMap[comp->id()] = nodeLabel;
            stream << "  " << nodeLabel << ": " << comp.get() << " type " << comp->typeName()
                   << " name " << comp->name() << "\n";
          };
        resource->visit(dumpNodes);
      }
      stream << "---\nArcs of " << resource->name() << "\n";
    }
  }

  /// Compile-time arc evaluation
  template<typename Impl, typename ArcTraits = typename Impl::Traits, typename ResourceType>
  void operator()(
    const Impl* arcs,
    const ResourceType* resource,
    std::ostream& stream,
    const Dump& self) const
  {
    std::string arcType = smtk::common::typeName<ArcTraits>();
    smtk::string::Token arcToken(arcType);
    if (
      !arcs || arcs->empty() ||
      (self.m_includeArcs.empty() &&
       self.m_excludeArcs.find(arcToken) != self.m_excludeArcs.end()) ||
      (!self.m_includeArcs.empty() &&
       self.m_includeArcs.find(arcToken) == self.m_includeArcs.end()))
    {
      // Skip arcs explicitly excluded (or not included if given a whitelist)
      return;
    }
    if (self.m_mimeType != "text/vnd.graphviz")
    {
      stream << "  " << arcType << "\n";
    }
    else
    {
      // To make a valid CSS class name from a namespace-qualified C++ name,
      // replace colons with underscores:
      std::string arcClass = arcType;
      std::replace(arcClass.begin(), arcClass.end(), ':', '_');
      stream << "  subgraph \"" << arcType << "\" {\n";
      stream << "    edge [class=\"" << arcClass << "\"";
      if (!ArcTraits::Directed::value)
      {
        stream << " dir=\"none\"";
      }
      auto colorIt = self.m_arcColors.find(arcType);
      if (colorIt != self.m_arcColors.end())
      {
        stream << " color=\"" << smtk::common::Color::floatRGBAToString(colorIt->second.data())
               << "\"";
      }
      stream << "]\n";
    }
    arcs->visitAllOutgoingNodes(
      resource, [&stream, &self](const typename ArcTraits::FromType* node) {
        int fromLabel = self.m_nodeMap[node->id()];
        std::size_t arcCount = 0;
        std::ostringstream line;
        if (self.m_mimeType != "text/vnd.graphviz")
        {
          line << "    " << node << ": ";
        }
        node->template outgoing<ArcTraits>().visit(
          [&fromLabel, &line, &arcCount, &stream, &self](const typename ArcTraits::ToType* other) {
            ++arcCount;
            int toLabel = self.m_nodeMap[other->id()];
            if (self.m_mimeType == "text/vnd.graphviz")
            {
              stream << "    " << fromLabel << " -> " << toLabel << "\n";
            }
            else
            {
              line << " " << toLabel;
            }
          });
        if (arcCount > 0 && self.m_mimeType != "text/vnd.graphviz")
        {
          stream << line.str() << "\n";
        }
      });
    if (self.m_mimeType == "text/vnd.graphviz")
    {
      stream << "  }\n";
    }
    stream << "\n";
  }

  /// Run-time arc evaluation
  template<typename ResourceType>
  void operator()(
    smtk::string::Token arcTypeName,
    const ArcImplementationBase& arcs,
    const ResourceType* resource,
    std::ostream& stream,
    const Dump& self) const
  {
    if (
      (self.m_includeArcs.empty() &&
       self.m_excludeArcs.find(arcTypeName) != self.m_excludeArcs.end()) ||
      (!self.m_includeArcs.empty() &&
       self.m_includeArcs.find(arcTypeName) == self.m_includeArcs.end()))
    {
      // Skip arcs explicitly excluded (or not included if given a whitelist)
      return;
    }
    if (self.m_mimeType != "text/vnd.graphviz")
    {
      stream << "  " << arcTypeName.data() << "\n";
    }
    else
    {
      // To make a valid CSS class name from a namespace-qualified C++ name,
      // replace colons with underscores:
      std::string arcClass = arcTypeName.data();
      std::replace(arcClass.begin(), arcClass.end(), ':', '_');
      stream << "  subgraph \"" << arcTypeName.data() << "\" {\n";
      stream << "    edge [class=\"" << arcClass << "\"";
      if (arcs.directionality() == Directionality::IsUndirected)
      {
        stream << " dir=\"none\"";
      }
      auto colorIt = self.m_arcColors.find(arcTypeName);
      if (colorIt != self.m_arcColors.end())
      {
        stream << " color=\"" << smtk::common::Color::floatRGBAToString(colorIt->second.data())
               << "\"";
      }
      stream << "]\n";
    }
    arcs.visitOutgoingNodes(
      resource, arcTypeName, [&arcTypeName, &stream, &self](const smtk::graph::Component* node) {
        int fromLabel = self.m_nodeMap[node->id()];
        std::size_t arcCount = 0;
        std::ostringstream line;
        if (self.m_mimeType != "text/vnd.graphviz")
        {
          line << "    " << node << ": ";
        }
        const_cast<smtk::graph::Component*>(node)
          ->outgoing(arcTypeName)
          .visit([&arcTypeName, &fromLabel, &line, &arcCount, &stream, &self](
                   const smtk::graph::Component* other) {
            ++arcCount;
            int toLabel = self.m_nodeMap[other->id()];
            if (self.m_mimeType == "text/vnd.graphviz")
            {
              stream << "    " << fromLabel << " -> " << toLabel << "\n";
            }
            else
            {
              line << " " << toLabel;
            }
            return smtk::common::Visit::Continue;
          });
        if (arcCount > 0 && self.m_mimeType != "text/vnd.graphviz")
        {
          stream << line.str() << "\n";
        }
        return smtk::common::Visit::Continue;
      });
    if (self.m_mimeType == "text/vnd.graphviz")
    {
      stream << "  }\n";
    }
    stream << "\n";
  }

  template<typename ResourceType>
  static void end(const ResourceType* resource, std::ostream& stream, const Dump& self)
  {
    if (!resource)
    {
      return;
    }
    if (self.m_mimeType == "text/vnd.graphviz")
    {
      stream << "}\n";
    }
    else // self.m_mimeType == "text/plain"
    {
      stream << "---\n";
    }
  }

  smtk::string::Token m_mimeType;
  std::set<smtk::string::Token> m_includeArcs;
  std::set<smtk::string::Token> m_excludeArcs;
  bool m_includeNodes = true;
  std::map<smtk::string::Token, std::array<double, 4>> m_arcColors;
  mutable std::map<smtk::common::UUID, int> m_nodeMap;
  mutable int m_nextNodeId = 1;
  static std::unique_ptr<std::array<double, 4>> s_backgroundColor;
};

} // namespace evaluators
} // namespace graph
} // namespace smtk

#endif // smtk_graph_arcs_Dump_h
