//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME smtkResource.cxx - Abstract base class for CMB resources
// .SECTION Description
// .SECTION See Also

#include "smtk/resource/Resource.h"

#include "smtk/resource/Manager.h"

#include "smtk/resource/filter/Filter.h"

#include "smtk/common/Paths.h"
#include "smtk/common/TypeName.h"
#include "smtk/common/UUIDGenerator.h"

#include "smtk/io/Logger.h"

namespace smtk
{
namespace resource
{

constexpr const char* const Resource::type_name;
const Resource::Index Resource::type_index = std::type_index(typeid(Resource)).hash_code();

Resource::Resource(const smtk::common::UUID& myID, ManagerPtr manager)
  : m_manager(manager)
  , m_id(myID)
  , m_clean(false)
  , m_links(this)
  , m_properties(this)
{
}

Resource::Resource(ManagerPtr manager)
  : Resource(smtk::common::UUIDGenerator::instance().random(), manager)
{
}

Resource::Resource(Resource&& rhs) noexcept
  : m_manager(std::move(rhs.m_manager))
  , m_id(std::move(rhs.m_id))
  , m_location(std::move(rhs.m_location))
  , m_name(std::move(rhs.m_name))
  , m_clean(std::move(rhs.m_clean))
  , m_links(std::move(rhs.m_links))
  , m_properties(std::move(rhs.m_properties))
  , m_queries(std::move(rhs.m_queries))
{
}

Resource::~Resource() = default;

Component* Resource::component(const smtk::common::UUID& compId) const
{
  return this->find(compId).get();
}

std::function<bool(const Component&)> Resource::queryOperation(
  const std::string& filterString) const
{
  // By default, return a filter that discriminates according to the default
  // property types.
  return smtk::resource::filter::Filter<>(filterString);
}

ComponentSet Resource::filter(const std::string& queryString) const
{
  // Construct a query operation from the query string
  auto queryOp = this->queryOperation(queryString);

  // Construct a component set to fill
  ComponentSet componentSet;

  // Visit each component and add it to the set if it satisfies the query
  smtk::resource::Component::Visitor visitor = [&](const ComponentPtr& component) {
    if (queryOp(*component))
    {
      componentSet.insert(component);
    }
  };

  this->visit(visitor);

  return componentSet;
}

bool Resource::isOfType(const Resource::Index& index) const
{
  return this->index() == index;
}

bool Resource::isOfType(const std::string& typeName) const
{
  return smtk::common::typeName<Resource>() == typeName;
}

int Resource::numberOfGenerationsFromBase(const std::string& typeName) const
{
  return (typeName == smtk::common::typeName<Resource>() ? 0 : std::numeric_limits<int>::lowest());
}

bool Resource::setId(const smtk::common::UUID& myId)
{
  if (myId == m_id)
  {
    return false;
  }

  Manager::Ptr mgr = m_manager.lock();
  if (mgr)
  {
    // If the resource's manager is set, then the resource is registered to a
    // manager. We need to change the resource's Id while ensuring we do not
    // break the indexing of the manager's collection of resources.
    common::UUID originalId(m_id);
    mgr->reviseId(SetId(originalId), SetId(myId));
  }
  else
  {
    // there is no resource manager tracking us, so let's just change the value.
    *const_cast<smtk::common::UUID*>(&m_id) = myId;
  }

  if (myId == m_id)
  {
    this->setClean(false);
    return true;
  }
  return false;
}

bool Resource::setLocation(const std::string& myLocation)
{
  if (myLocation == m_location)
  {
    return false;
  }

  Manager::Ptr mgr = m_manager.lock();
  if (mgr)
  {
    std::string originalLocation(m_location);
    mgr->reviseLocation(m_id, SetLocation(originalLocation), SetLocation(myLocation));
  }
  else
  {
    // there is no resource manager tracking us, so let's just change the value.
    *const_cast<std::string*>(&m_location) = myLocation;
  }

  if (myLocation == m_location)
  {
    this->setClean(false);
    return true;
  }
  return false;
}

std::string Resource::name() const
{
  if (m_name.empty())
  {
    return smtk::common::Paths::stem(m_location);
  }
  return m_name;
}

bool Resource::setName(const std::string& name)
{
  if (name == m_name)
  {
    return false;
  }
  m_name = name;
  this->setClean(false);
  return true;
}

void Resource::setClean(bool state)
{
  if (m_clean == state)
  {
    return;
  }
  m_clean = state;
  auto mgr = this->manager();
  if (mgr)
  {
    mgr->observers()(*this, EventType::MODIFIED);
  }
}

} // namespace resource
} // namespace smtk
