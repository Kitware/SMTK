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

#include "smtk/common/UUIDGenerator.h"

#include "smtk/resource/Manager.h"

namespace smtk
{
namespace resource
{

Resource::Resource(const smtk::common::UUID& myID, Manager* manager)
  : m_id(myID)
  , m_location()
  , m_manager(manager)
{
}

Resource::Resource(Manager* manager)
  : m_id(smtk::common::UUIDGenerator::instance().random())
  , m_location()
  , m_manager(manager)
{
}

Resource::~Resource()
{
}

std::string Resource::uniqueName() const
{
  if (m_manager)
  {
    // if the resource's manager is set, then the resource is registered to a
    // manager. The resource metadata has a unique name for this resource type,
    // so we return this name.
    auto metadata = m_manager->metadata().get<IndexTag>().find(this->index());
    if (metadata != m_manager->metadata().get<IndexTag>().end())
    {
      return metadata->uniqueName();
    }
  }

  // either this resource is not registered to a manager or it does not have a
  // unique name registered to it. Simply return the class name.
  return this->classname();
}

bool Resource::setId(const smtk::common::UUID& myId)
{
  if (m_manager)
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
    ResourcesById& resources = m_manager->resources().get<IdTag>();
    ResourcesById::iterator resourceIt = resources.find(this->m_id);

    // try to modify the id, restore it in case of collisions
    smtk::common::UUID originalId = this->m_id;
    resources.modify(resourceIt, SetId(myId), SetId(originalId));
  }
  else
  {
    // there is no resource manager tracking us, so let's just change the value.
    *const_cast<smtk::common::UUID*>(&m_id) = myId;
  }

  return myId == this->m_id;
}

bool Resource::setLocation(const std::string& myLocation)
{
  if (m_manager)
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

    typedef Container::index<LocationTag>::type ResourcesByLocation;
    ResourcesByLocation& resources = m_manager->resources().get<LocationTag>();
    ResourcesByLocation::iterator resourceIt =
      m_manager->resources().get<LocationTag>().find(this->m_location);

    // modify the location
    resources.modify(resourceIt, SetLocation(myLocation));
  }
  else
  {
    // there is no resource manager tracking us, so let's just change the value.
    *const_cast<std::string*>(&m_location) = myLocation;
  }

  return myLocation == this->m_location;
}

std::string Resource::type2String(Resource::Type t)
{
  switch (t)
  {
    case ATTRIBUTE:
      return "attribute";
    case MODEL:
      return "model";
    case MESH:
      return "mesh";
    default:
      return "";
  }
  return "Error!";
}

Resource::Type Resource::string2Type(const std::string& s)
{
  if (s == "attribute")
  {
    return ATTRIBUTE;
  }
  if (s == "model")
  {
    return MODEL;
  }
  if (s == "mesh")
  {
    return MESH;
  }
  return NUMBER_OF_TYPES;
}

} // namespace resource
} // namespace smtk
