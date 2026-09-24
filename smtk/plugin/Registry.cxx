//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/plugin/Registry.h"

namespace smtk
{
namespace plugin
{
namespace detail
{
ManagerCount ManagerCount::m_instance;

struct ManagerCount::Internals
{
  std::map<std::pair<void*, std::size_t>, std::size_t> m_ManagerMap;
  std::map<
    std::weak_ptr<void>,
    std::map<std::size_t, std::size_t>,
    std::owner_less<std::weak_ptr<void>>>
    m_OwnerMap;
};

ManagerCount::ManagerCount()
  : m_internals(new Internals())
{
}

ManagerCount::~ManagerCount()
{
  delete m_internals;
}

std::size_t& ManagerCount::operator[](const std::pair<void*, std::size_t>& key)
{
  return m_internals->m_ManagerMap[key];
}
std::size_t& ManagerCount::count(const std::weak_ptr<void>& manager, std::size_t registrar)
{
  return m_internals->m_OwnerMap[manager][registrar];
}
} // namespace detail
} // namespace plugin
} // namespace smtk
