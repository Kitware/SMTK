//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/Logger.h"

#include "smtk/markup/Resource.h"

#include "smtk/markup/IdSpace.h"
#include "smtk/markup/queries/SelectionFootprint.h"
// #include "smtk/markup/arcs/ChildrenAs.txx"
// #include "smtk/markup/arcs/ParentsAs.txx"

#include "smtk/resource/query/Queries.h"

#include "smtk/common/Paths.h"
#include "smtk/common/StringUtil.h"

namespace smtk
{
namespace markup
{

namespace
{
using QueryTypes = std::tuple<SelectionFootprint>;
}

DomainFactory Resource::s_domainFactory;

Resource::Resource(const smtk::common::UUID& uid, smtk::resource::ManagerPtr manager)
  : Superclass(uid, manager)
{
  this->initialize();
}

Resource::Resource(smtk::resource::ManagerPtr manager)
  : Superclass(manager)
{
  this->initialize();
}

bool Resource::setLocation(const std::string& location)
{
  std::string prev = this->location();
  if (!this->Superclass::setLocation(location))
  {
    return false;
  }
  // Erase resource-relative "output" URL locations; they will be set by markup::Write.
  (void)prev;
  const auto& nodesByType = m_nodes.get<detail::TypeNameTag>();
  auto it = nodesByType.lower_bound(smtk::common::typeName<URL>());
  if (it == nodesByType.end())
  {
    return true;
  }
  auto lastNode = nodesByType.upper_bound(smtk::common::typeName<URL>());
  smtk::string::Token empty;
  for (; it != lastNode; ++it)
  {
    auto url = std::static_pointer_cast<smtk::markup::URL>(*it);
    if (!url->outgoing<arcs::URLsToData>().empty())
    {
      if (smtk::common::Paths::isRelative(url->location().data()))
      {
        url->setLocation(empty);
      }
    }
  }
  return true;
}

std::function<bool(const smtk::resource::Component&)> Resource::queryOperation(
  const std::string& query) const
{
  return smtk::resource::filter::Filter<smtk::graph::filter::Grammar>(query);
}

void Resource::initialize()
{
  using namespace smtk::string::literals; // for ""_token
  this->queries().registerQueries<QueryTypes>();
  std::vector<std::shared_ptr<smtk::markup::IdSpace>> domains{
    // Discrete models have ID namespaces for points and cells.
    std::make_shared<smtk::markup::IdSpace>("points"_token),
    std::make_shared<smtk::markup::IdSpace>("cells"_token),
    // Side sets of discrete models also require an ID namespace and map
    // for moving between a "side ID" and a boundary of the reference element
    // (note that sides may be of any dimension lower than the cell, not just
    // a single dimension lower).
    // We provide an IdSpace for each cell shape VTK supports.
    std::make_shared<smtk::markup::IdSpace>("sides(hexahedron)"_token),
    std::make_shared<smtk::markup::IdSpace>("sides(tetrahedron)"_token),
    std::make_shared<smtk::markup::IdSpace>("sides(wedge)"_token),
    std::make_shared<smtk::markup::IdSpace>("sides(pyramid)"_token),
    std::make_shared<smtk::markup::IdSpace>("sides(quadrilateral)"_token),
    std::make_shared<smtk::markup::IdSpace>("sides(triangle)"_token),
    std::make_shared<smtk::markup::IdSpace>("sides(line)"_token),
    std::make_shared<smtk::markup::IdSpace>("sides(vertex)"_token)
  };
  for (const auto& domain : domains)
  {
    m_domains.insert(domain->name(), domain);
  }
}

} // namespace markup
} // namespace smtk
