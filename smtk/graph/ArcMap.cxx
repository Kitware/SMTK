//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/graph/ArcMap.h"
#include "smtk/graph/ResourceBase.h"
#include "smtk/graph/RuntimeArc.h"

#include "smtk/resource/CopyOptions.h"

#include "smtk/io/Logger.h"

namespace smtk
{
namespace graph
{

std::unordered_set<smtk::string::Token> ArcMap::runtimeBaseTypes() const
{
  std::unordered_set<smtk::string::Token> result;
  for (const auto& entry : m_runtimeArcs)
  {
    result.insert(entry.first);
  }
  return result;
}

const std::unordered_set<smtk::string::Token>& ArcMap::runtimeTypeNames(
  smtk::string::Token baseType) const
{
  auto it = m_runtimeArcs.find(baseType);
  if (it == m_runtimeArcs.end())
  {
    static std::unordered_set<smtk::string::Token> dummy;
    return dummy;
  }
  return it->second;
}

bool ArcMap::insertRuntimeArcType(
  smtk::graph::ResourceBase* resource,
  smtk::string::Token arcType,
  std::unordered_set<smtk::string::Token> fromNodeSpecs,
  std::unordered_set<smtk::string::Token> toNodeSpecs,
  Directionality isDirected)
{
  bool didInsert = false;
  smtk::string::Token implementation;
  if (isDirected)
  {
    implementation = smtk::common::typeName<ArcImplementation<RuntimeArc<IsDirected>>>();
    RuntimeArc<IsDirected> arcSpec(arcType, fromNodeSpecs, toNodeSpecs, resource);
    auto arcData = std::make_shared<ArcImplementation<RuntimeArc<IsDirected>>>(arcSpec);
    didInsert = m_data.emplace(arcType, arcData).second;
  }
  else
  {
    implementation = smtk::common::typeName<ArcImplementation<RuntimeArc<IsUndirected>>>();
    RuntimeArc<IsUndirected> arcSpec(arcType, fromNodeSpecs, toNodeSpecs, resource);
    auto arcData = std::make_shared<ArcImplementation<RuntimeArc<IsUndirected>>>(arcSpec);
    didInsert = m_data.emplace(arcType, arcData).second;
  }
  if (didInsert)
  {
    m_runtimeArcs[implementation].insert(arcType);
    m_types.insert(arcType);
  }
  else
  {
    smtkWarningMacro(
      smtk::io::Logger::instance(), "Failed to insert \"" << arcType.data() << "\" arc type.");
  }
  return didInsert;
}

void ArcMap::copyArcs(
  const smtk::graph::ResourceBase* source,
  // const ArcMap& source,
  smtk::resource::CopyOptions& options,
  smtk::graph::ResourceBase* target)
{
  // I. Add runtime arc types.
  for (const auto& baseArcEntry : source->arcs().m_runtimeArcs)
  {
    auto baseIt = m_runtimeArcs.find(baseArcEntry.first);
    if (baseIt == m_runtimeArcs.end())
    {
      std::unordered_set<smtk::string::Token> empty;
      m_runtimeArcs[baseArcEntry.first] = empty;
      baseIt = m_runtimeArcs.find(baseArcEntry.first);
    }
    if (baseIt->first != smtk::common::typeName<ArcImplementationBase>())
    {
      smtkWarningMacro(
        options.log(), "Unhandled arc base-type \"" << baseIt->first.data() << "\". Skipping.");
      continue;
    }
    for (const auto& runtimeArcEntry : baseArcEntry.second)
    {
      try
      {
        const auto& sourceArcImpl =
          source->arcs().getRuntime<ArcImplementationBase>(runtimeArcEntry);
        auto runtimeIt = baseIt->second.find(runtimeArcEntry);
        if (runtimeIt == baseIt->second.end())
        {
          bool didAdd = this->insertRuntimeArcType(
            target,
            runtimeArcEntry,
            sourceArcImpl.fromTypes(),
            sourceArcImpl.toTypes(),
            sourceArcImpl.directionality());
          if (!didAdd)
          {
            smtkErrorMacro(
              options.log(), "Could not add runtime type \"" << runtimeArcEntry.data() << "\".");
          }
        }
        else
        {
          // TODO: Verify that sourceArcImpl and the target implementation match.
        }
      }
      catch (smtk::resource::query::BadTypeError& err)
      {
        (void)err;
        smtkErrorMacro(
          options.log(),
          "Could not fetch source storage for \"" << runtimeArcEntry.data() << "\".");
      }
    }
  }
  // II. Copy arcs
  for (const auto& sourceEntry : source->arcs().m_data)
  {
    // Skip arc types that are implicit:
    if (!sourceEntry.second->explicitStorage())
    {
      continue;
    }

    sourceEntry.second->visitOutgoingNodes(
      source, sourceEntry.first, [&](const Component* from) -> smtk::common::Visit {
        auto* targetFrom = options.targetObjectFromSourceId<smtk::graph::Component>(from->id());
        if (!targetFrom || options.shouldOmitId(from->id()))
        {
          return smtk::common::Visit::Continue;
        }
        auto targetOut = targetFrom->outgoing(sourceEntry.first);
        from->outgoing(sourceEntry.first).visit([&](const Component* to) {
          auto* targetTo = options.targetObjectFromSourceId<smtk::graph::Component>(to->id());
          if (!targetTo || options.shouldOmitId(to->id()))
          {
            return smtk::common::Visit::Continue;
          }
          if (!targetOut.connect(targetTo))
          {
            smtkErrorMacro(
              options.log(),
              "Could not connect " << targetFrom->id() << " to " << targetTo->id() << ".");
          }
          return smtk::common::Visit::Continue;
        });
        return smtk::common::Visit::Continue;
      });
  }
}

} // namespace graph
} // namespace smtk
