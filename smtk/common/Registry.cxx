//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/common/Registry.h"

namespace smtk
{
namespace common
{
namespace detail
{
ManagerCount ManagerCount::m_instance;

ManagerCount& ManagerCount::instance()
{
  return m_instance;
}

std::size_t& ManagerCount::operator[](const std::pair<void*, std::size_t>& key)
{
  return m_ManagerMap[key];
}
}
}
}
