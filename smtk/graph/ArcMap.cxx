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

} // namespace graph
} // namespace smtk
