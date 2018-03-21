//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "Set.h"
#include "Wrapper.h"
#include <iostream>

namespace smtk
{
namespace resource
{

typedef Set::Role Role;
typedef Set::State State;

Set::Set()
{
}

Set::~Set()
{
  // Release elements in m_resourceMap
  std::map<std::string, Wrapper*>::iterator iter;
  for (iter = m_resourceMap.begin(); iter != m_resourceMap.end(); iter++)
  {
    delete iter->second;
  }
}

bool Set::add(ResourcePtr resource, std::string id, std::string link, Role role)
{
  // Check that id not already in use
  Wrapper* wrapper = this->getWrapper(id);
  if (wrapper != NULL)
  {
    std::cerr << "ERROR: Id " << id << " already in use" << std::endl;
    return false;
  }

  // Instantiate wrapper
  wrapper = new Wrapper();
  wrapper->resource = resource;
  wrapper->role = role;
  wrapper->state = LOADED;
  wrapper->id = id;
  wrapper->link = link;
  m_resourceMap[id] = wrapper;
  m_resourceIds.push_back(id);

  return true;
}

// Add resource info but *not* the resource itself
// For links and error-loading cases
bool Set::addInfo(const std::string id, Role role, State state, std::string link)
{
  // Check that id not already in use
  Wrapper* wrapper = this->getWrapper(id);
  if (wrapper != NULL)
  {
    std::cerr << "ERROR: Id " << id << " already in use" << std::endl;
    return false;
  }

  wrapper = new Wrapper();
  wrapper->role = role;
  wrapper->state = state;
  wrapper->id = id;
  wrapper->link = link;
  m_resourceMap[id] = wrapper;
  m_resourceIds.push_back(id);

  return true;
}

/// Remove (not unload, but instead entirely delete) the resource and all its information from the set.
bool Set::remove(const std::string& id)
{
  std::map<std::string, Wrapper*>::iterator mit = m_resourceMap.find(id);
  if (mit != m_resourceMap.end())
  {
    std::vector<std::string>::size_type ii;
    std::vector<std::string>::size_type nn = m_resourceIds.size();
    for (ii = 0; ii < nn; ++ii)
    {
      if (m_resourceIds[ii] == id)
      {
        m_resourceIds.erase(m_resourceIds.begin() + ii);
        break;
      }
    }
    delete mit->second;
    mit->second = nullptr;
    m_resourceMap.erase(mit);
    return true;
  }
  return false;
}

std::size_t Set::numberOfResources() const
{
  return m_resourceIds.size();
}

const std::vector<std::string> Set::resourceIds() const
{
  return m_resourceIds;
}

bool Set::resourceInfo(std::string id, Role& role, State& state, std::string& link) const
{
  // Get wrapper from resource map
  Wrapper* wrapper = this->getWrapper(id);
  if (wrapper == NULL)
  {
    std::cerr << "Id " << id << " not found" << std::endl;
    return false;
  }

  role = wrapper->role;
  state = wrapper->state;
  link = wrapper->link;
  return true;
}

bool Set::get(std::string id, ResourcePtr& resource) const
{
  // Get wrapper from resource map
  Wrapper* wrapper = this->getWrapper(id);
  if (wrapper == NULL)
  {
    std::cerr << "Id " << id << " not found" << std::endl;
    return false;
  }

  resource = wrapper->resource;
  return true;
}

Wrapper* Set::getWrapper(std::string id) const
{
  // Get wrapper from resource map
  std::map<std::string, Wrapper*>::const_iterator iter = m_resourceMap.find(id);
  if (iter == m_resourceMap.end())
  {
    return NULL;
  }

  return iter->second;
}

// Converts State to string
std::string Set::state2String(State state)
{
  std::string s; // return value
  switch (state)
  {
    case Set::NOT_LOADED:
      s = "not-loaded";
      break;
    case Set::LOADED:
      s = "loaded";
      break;
    case Set::LOAD_ERROR:
      s = "load-error";
      break;
    default:
      s = "unknown-state";
      break;
  }
  return s;
}

// Converts Role to string
std::string Set::role2String(Role role)
{
  std::string s; // return value
  switch (role)
  {
    case Set::TEMPLATE:
      s = "template";
      break;
    case Set::SCENARIO:
      s = "scenario";
      break;
    case Set::INSTANCE:
      s = "instance";
      break;
    case Set::MODEL_RESOURCE:
      s = "model";
      break;
    case Set::AUX_GEOM_RESOURCE:
      s = "auxiliary geometry";
      break;
    default:
      s = "unknown-role";
      break;
  }
  return s;
}

// Converts string to Role
Role Set::string2Role(const std::string s)
{
  Role role = Set::NOT_DEFINED;
  if (s == "template")
  {
    role = Set::TEMPLATE;
  }
  else if (s == "scenario")
  {
    role = Set::SCENARIO;
  }
  else if (s == "instance")
  {
    role = Set::INSTANCE;
  }
  else if (s == "model")
  {
    role = Set::MODEL_RESOURCE;
  }
  else if (s == "auxiliary geometry" || s == "aux geom")
  {
    role = Set::AUX_GEOM_RESOURCE;
  }
  else
  {
    std::cerr << "Unrecognized role string " << role << std::endl;
  }
  return role;
}

// Set & Get methods for m_linkStartPath
void Set::setLinkStartPath(const std::string s)
{
  m_linkStartPath = s;
}

//----------------------------------------------------------------------------
std::string Set::linkStartPath() const
{
  return m_linkStartPath;
}

} // namespace resource
} // namespace smtk
