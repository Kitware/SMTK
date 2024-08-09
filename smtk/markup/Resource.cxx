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

#include "smtk/resource/CopyOptions.h"
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

std::shared_ptr<smtk::resource::Resource> Resource::clone(
  smtk::resource::CopyOptions& options) const
{
  auto result = std::shared_ptr<Resource>(new Resource);
  this->prepareClone(options, result);
  return result;
}

bool Resource::copyInitialize(
  const std::shared_ptr<const smtk::resource::Resource>& source,
  smtk::resource::CopyOptions& options)
{
  // Copy the domains from the source resource but without adding
  // ID assignments (as components are copied, they will request
  // assignments that should be valid and consistent as long as
  // the source resource is properly defined).
  auto markupSource = std::dynamic_pointer_cast<const smtk::markup::Resource>(source);
  if (!markupSource)
  {
    smtkErrorMacro(options.log(), "Source resource must be a markup resource.");
    return false;
  }
  for (const auto& domainName : markupSource->domains().keys())
  {
    if (!domainName.hasData())
    {
      smtkErrorMacro(
        options.log(), "Domain " << domainName.id() << " has no associated string; cannot create.");
      continue;
    }
    auto sourceDomain = markupSource->domains().find(domainName);
    if (!sourceDomain)
    {
      continue;
    }
    auto targetDomain = this->domains().find(domainName);
    if (!targetDomain)
    {
      auto space = Resource::domainFactory().makeFromName(domainName.data());
      if (space)
      {
        this->domains().insert(domainName, space);
      }
      else
      {
        smtkErrorMacro(
          options.log(), "Could not create a \"" << domainName.data() << "\" for target.");
      }
    }
  }
  return this->Superclass::copyInitialize(source, options);
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
