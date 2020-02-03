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

#include "smtk/common/Paths.h"
#include "smtk/common/TypeName.h"
#include "smtk/common/UUIDGenerator.h"

namespace smtk
{
namespace resource
{

Resource::Resource(const smtk::common::UUID& myID, ManagerPtr manager)
  : m_id(myID)
  , m_clean(false)
  , m_links(this)
  , m_properties(this)
  , m_manager(manager)
{
}

Resource::Resource(ManagerPtr manager)
  : Resource(smtk::common::UUIDGenerator::instance().random(), manager)
{
}

Resource::~Resource() = default;

ComponentSet Resource::find(const std::string& queryString) const
{
  //  return this->findAs<ComponentSet>(queryString);
  // Construct a query operation from the query string
  auto queryOp = this->queryOperation(queryString);

  // Construct a component set to fill
  ComponentSet componentSet;

  // Visit each component and add it to the set if it satisfies the query
  smtk::resource::Component::Visitor visitor = [&](const ComponentPtr& component) {
    if (queryOp(component))
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
    // if the resource's manager is set, then the resource is registered to a
    // manager. We need to change the resource's Id while ensuring we do not
    // break the indexing of the manager's collection of resources.
    struct SetId
    {
      SetId(const smtk::common::UUID& id)
        : m_id(id)
      {
      }

      void operator()(ResourcePtr& resource) { resource->m_id = m_id; }

      const smtk::common::UUID& m_id;
    };

    typedef Container::index<IdTag>::type ResourcesById;
    ResourcesById& resources = mgr->resources().get<IdTag>();
    ResourcesById::iterator resourceIt = resources.find(m_id);

    // try to modify the id, restore it in case of collisions
    smtk::common::UUID originalId = m_id;
    resources.modify(resourceIt, SetId(myId), SetId(originalId));
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
    // if the resource's manager is set, then the resource is registered to a
    // manager. We need to change the resource's location while ensuring we do
    // not break the indexing of the manager's collection of resources.
    struct SetLocation
    {
      SetLocation(const std::string& location)
        : m_location(location)
      {
      }

      void operator()(ResourcePtr& resource) { resource->m_location = m_location; }

      const std::string& m_location;
    };

    // We search for the resource by ID, since locations are not unique.
    typedef Container::index<IdTag>::type ResourcesById;
    ResourcesById& resources = mgr->resources().get<IdTag>();
    ResourcesById::iterator resourceIt = resources.find(m_id);

    // modify the location
    resources.modify(resourceIt, SetLocation(myLocation));
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
