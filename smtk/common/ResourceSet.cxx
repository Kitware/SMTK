//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "ResourceSet.h"
#include <iostream>

namespace smtk {
  namespace common {

typedef ResourceSet::ResourceRole ResourceRole;
typedef ResourceSet::ResourceState ResourceState;


// Simple container for single Resource plus meta data
struct ResourceWrapper
{
  ResourcePtr resource;
  Resource::Type type;
  ResourceRole role;
  ResourceState state;
  std::string id;
  std::string link;
};


//----------------------------------------------------------------------------
ResourceSet::ResourceSet()
{
}

//----------------------------------------------------------------------------
ResourceSet::~ResourceSet()
{
  // Release elements in m_resourceMap
  std::map<std::string, ResourceWrapper*>::iterator iter;
  for (iter=m_resourceMap.begin(); iter != m_resourceMap.end(); iter++)
    {
    delete iter->second;
    }
}

//----------------------------------------------------------------------------
bool
ResourceSet::
addResource(ResourcePtr resource,
            std::string id,
            std::string link,
            ResourceRole role)
{
  // Check that id not already in use
  ResourceWrapper *wrapper = this->getWrapper(id);
  if (wrapper != NULL)
    {
    std::cerr << "ERROR: Id " << id << " already in use" << std::endl;
    return false;
    }

  // Require attribute resources to specify role
  if (resource->resourceType() == Resource::ATTRIBUTE &&
      role == NOT_DEFINED)
    {
    std::cerr << "ERROR: Role not specified for attribute resource " <<
      id << std::endl;
    return false;
    }

  // Instantiate wrapper
  wrapper = new ResourceWrapper();
  wrapper->resource = resource;
  wrapper->type = resource->resourceType();
  wrapper->role = role;
  wrapper->state = LOADED;
  wrapper->id = id;
  wrapper->link = link;
  m_resourceMap[id] = wrapper;
  m_resourceIds.push_back(id);

  return true;
}

//----------------------------------------------------------------------------
// Add resource info but *not* the resource itself
// For links and error-loading cases
bool
ResourceSet::
addResourceInfo(const std::string id,
                Resource::Type type,
                ResourceRole role,
                ResourceState state,
                std::string link)
{
  // Check that id not already in use
  ResourceWrapper *wrapper = this->getWrapper(id);
  if (wrapper != NULL)
    {
    std::cerr << "ERROR: Id " << id << " already in use" << std::endl;
    return false;
    }

  // Attribute resources must specify role
  if (type == Resource::ATTRIBUTE && role == NOT_DEFINED)
    {
    std::cerr << "ERROR: Role not specified for attribute resource " <<
      id << std::endl;
    return false;
    }

  wrapper = new ResourceWrapper();
  wrapper->type = type;
  wrapper->role = role;
  wrapper->state = state;
  wrapper->id = id;
  wrapper->link = link;
  m_resourceMap[id] = wrapper;
  m_resourceIds.push_back(id);

  return true;
}

//----------------------------------------------------------------------------
unsigned int
ResourceSet::
numberOfResources() const
{
  return m_resourceIds.size();
}

//----------------------------------------------------------------------------
const std::vector<std::string>
ResourceSet::
resourceIds() const
{
  return m_resourceIds;
}

//----------------------------------------------------------------------------
bool
ResourceSet::
resourceInfo(std::string id,
             Resource::Type& type,
             ResourceRole& role,
             ResourceState& state,
             std::string& link) const
{
  // Get wrapper from resource map
  ResourceWrapper *wrapper = this->getWrapper(id);
  if (wrapper == NULL)
    {
    std::cerr << "Id " << id << " not found" << std::endl;
    return false;
    }

    type = wrapper->type;
    role = wrapper->role;
    state = wrapper->state;
    link = wrapper->link;
    return true;
}

//----------------------------------------------------------------------------
bool
ResourceSet::
get(std::string id, ResourcePtr& resource) const
{
  // Get wrapper from resource map
  ResourceWrapper *wrapper = this->getWrapper(id);
  if (wrapper == NULL)
    {
    std::cerr << "Id " << id << " not found" << std::endl;
    return false;
    }

    resource = wrapper->resource;
    return true;
}

//----------------------------------------------------------------------------
ResourceWrapper *
ResourceSet::
getWrapper(std::string id) const
{
  // Get wrapper from resource map
  std::map<std::string, ResourceWrapper*>::const_iterator iter =
    m_resourceMap.find(id);
  if (iter == m_resourceMap.end())
    {
    return NULL;
    }

  return iter->second;
}

//----------------------------------------------------------------------------
// Converts ResourceState to string
std::string
ResourceSet::
state2String(ResourceState state)
{
  std::string s;  // return value
  switch (state)
    {
    case ResourceSet::NOT_LOADED: s = "not-loaded"; break;
    case ResourceSet::LOADED:     s = "loaded";     break;
    case ResourceSet::LOAD_ERROR: s = "load-error"; break;
    default: s = "unknown-state"; break;
    }
  return s;
}

//----------------------------------------------------------------------------
// Converts ResourceRole to string
std::string
ResourceSet::
role2String(ResourceRole role)
{
  std::string s;  // return value
  switch (role)
    {
    case ResourceSet::TEMPLATE: s = "template"; break;
    case ResourceSet::SCENARIO: s = "scenario"; break;
    case ResourceSet::INSTANCE: s = "instance"; break;
    default: s = "unknown-role"; break;
    }
  return s;
}

//----------------------------------------------------------------------------
// Converts string to ResourceRole
ResourceRole
ResourceSet::
string2Role(const std::string s)
{
  ResourceRole role = ResourceSet::NOT_DEFINED;
  if (s == "template")
    {
    role = ResourceSet::TEMPLATE;
    }
  else if (s == "scenario")
    {
    role = ResourceSet::SCENARIO;
    }
  else if (s == "instance")
    {
    role = ResourceSet::INSTANCE;
    }
  else
    {
    std::cerr << "Unrecognized role string " << role << std::endl;
    }
  return role;
}

//----------------------------------------------------------------------------
// Set & Get methods for m_linkStartPath
void
ResourceSet::
setLinkStartPath(const std::string s)
{
  m_linkStartPath = s;
}

std::string
ResourceSet::
linkStartPath() const
{
  return m_linkStartPath;
}

  } // namespace common
} // namespace smtk
