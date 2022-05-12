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
