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

#include "smtk/string/Token.h"

#include "smtk/common/Color.h"

#include "nlohmann/json.hpp"

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

  template<typename ResourcePtr>
  static void begin(ResourcePtr resource, std::ostream& stream, const Dump& self)
  {
    if (!resource)
    {
      return;
    }
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
          [&stream](const smtk::resource::ComponentPtr& comp) {
            int nodeLabel = static_cast<int>(comp->id().begin()[0]);
            stream << "  " << nodeLabel << " [class=\"" << comp->typeName() << "\", label=\""
                   << comp->name() << "\"]\n";
          };
        resource->visit(dumpNodes);
        stream << "\n";
      }
    }
    else // self.m_mimeType == "text/plain"
    {
      if (self.m_includeNodes)
      {
        stream << "---\nNodes of " << resource->name() << "\n";
        std::function<void(const std::shared_ptr<smtk::resource::Component>&)> dumpNodes =
          [&stream](const smtk::resource::ComponentPtr& comp) {
            stream << "  " << comp.get() << " type " << comp->typeName() << " name " << comp->name()
                   << "\n";
          };
        resource->visit(dumpNodes);
      }
      stream << "---\nArcs of " << resource->name() << "\n";
    }
  }

  template<typename Impl, typename ArcTraits = typename Impl::Traits, typename ResourcePtr>
  void operator()(const Impl* arcs, ResourcePtr resource, std::ostream& stream, const Dump& self)
    const
  {
    std::string arcType = smtk::common::typeName<ArcTraits>();
    smtk::string::Token arcToken(arcType);
    if (
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
      stream << "  subgraph \"" << arcType << "\" {\n";
      if (!ArcTraits::Directed::value)
      {
        stream << "    edge [dir=\"none\"]\n";
      }
      auto colorIt = m_arcColors.find(arcType);
      if (colorIt != m_arcColors.end())
      {
        stream << "    edge [color=\""
               << smtk::common::Color::floatRGBAToString(colorIt->second.data()) << "\"]\n";
      }
    }
    arcs->visitAllOutgoingNodes(
      resource, [&stream, &self](const typename ArcTraits::FromType* node) {
        int fromLabel = static_cast<int>(node->id().begin()[0]);
        std::size_t arcCount = 0;
        std::ostringstream line;
        if (self.m_mimeType != "text/vnd.graphviz")
        {
          line << "    " << node << ": ";
        }
        node->template outgoing<ArcTraits>().visit(
          [&fromLabel, &line, &arcCount, &stream, &self](const typename ArcTraits::ToType* other) {
            ++arcCount;
            if (self.m_mimeType == "text/vnd.graphviz")
            {
              int toLabel = static_cast<int>(other->id().begin()[0]);
              stream << "    " << fromLabel << " -> " << toLabel << "\n";
            }
            else
            {
              line << " " << other;
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

  template<typename ResourcePtr>
  static void end(ResourcePtr resource, std::ostream& stream, const Dump& self)
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
  static std::unique_ptr<std::array<double, 4>> s_backgroundColor;
};

} // namespace evaluators
} // namespace graph
} // namespace smtk

#endif // smtk_graph_arcs_Dump_h
