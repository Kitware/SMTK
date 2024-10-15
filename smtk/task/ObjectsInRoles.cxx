//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/ObjectsInRoles.h"

#include "smtk/resource/PersistentObject.h"

namespace smtk
{
namespace task
{

bool ObjectsInRoles::addObject(smtk::resource::PersistentObject* object, smtk::string::Token role)
{
  if (!object || !role.valid())
  {
    return false;
  }
  return m_data[role].insert(object).second;
}

bool ObjectsInRoles::removeObject(
  smtk::resource::PersistentObject* object,
  smtk::string::Token role)
{
  if (!object)
  {
    return false;
  }
  if (role.valid())
  {
    auto it = m_data.find(role);
    if (it == m_data.end())
    {
      return false;
    }
    bool didRemove = it->second.erase(object) > 0;
    if (it->second.empty())
    {
      m_data.erase(it);
    }
    return didRemove;
  }
  std::size_t numRemoved = 0;
  RoleMap::iterator next;
  for (auto it = m_data.begin(); it != m_data.end(); it = next)
  {
    next = it;
    ++next;
    auto count = it->second.erase(object);
    if (it->second.empty())
    {
      m_data.erase(it);
    }
    numRemoved += count;
  }
  return numRemoved > 0;
}

bool ObjectsInRoles::merge(const PortData* other)
{
  if (const auto* objectData = dynamic_cast<const ObjectsInRoles*>(other))
  {
    for (const auto& otherEntry : objectData->data())
    {
      m_data[otherEntry.first].insert(otherEntry.second.begin(), otherEntry.second.end());
    }
    return true;
  }
  return false;
}

} // namespace task
} // namespace smtk
