//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/common/RuntimeTypeContainer.h"

namespace smtk
{
namespace common
{

RuntimeTypeContainer::RuntimeTypeContainer(const RuntimeTypeContainer& other)
  : TypeContainer(other)
{
  m_runtimeObjects = other.m_runtimeObjects;
}

RuntimeTypeContainer& RuntimeTypeContainer::operator=(const RuntimeTypeContainer& other)
{
  for (const auto& entry : other.m_container)
  {
    m_container.emplace(std::make_pair(entry.first, entry.second->clone()));
  }
  m_runtimeObjects = other.m_runtimeObjects;

  return *this;
}

std::unordered_set<smtk::string::Token> RuntimeTypeContainer::runtimeBaseTypes()
{
  std::unordered_set<smtk::string::Token> result;
  for (const auto& entry : m_runtimeObjects)
  {
    result.insert(entry.first);
  }
  return result;
}

std::unordered_set<smtk::string::Token> RuntimeTypeContainer::runtimeTypeNames(
  smtk::string::Token baseType)
{
  auto it = m_runtimeObjects.find(baseType);
  if (it == m_runtimeObjects.end())
  {
    std::unordered_set<smtk::string::Token> empty;
    return empty;
  }
  return it->second;
}

} // namespace common
} // namespace smtk
