//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/common/TypeContainer.h"

namespace smtk
{
namespace common
{

TypeContainer::~TypeContainer() = default;

TypeContainer::TypeContainer(const TypeContainer& other)
{
  *this = other;
}

TypeContainer& TypeContainer::operator=(const TypeContainer& other)
{
  for (const auto& entry : other.m_container)
  {
    m_container.emplace(entry.first, entry.second->clone());
  }

  return *this;
}
} // namespace common
} // namespace smtk
