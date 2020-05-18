//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/view/OperationIcons.h"

namespace smtk
{
namespace view
{

void OperationIcons::registerDefaultIconConstructor(IconConstructor&& functor)
{
  m_defaultIconConstructor = functor;
}

std::string OperationIcons::createIcon(
  const std::string& operationName,
  const std::string& secondaryColor) const
{
  auto nameIt = m_indices.find(operationName);
  FunctorMap::const_iterator ctorIt;
  if (nameIt == m_indices.end() || ((ctorIt = m_functors.find(nameIt->second)) == m_functors.end()))
  {
    if (m_defaultIconConstructor)
    {
      return m_defaultIconConstructor(secondaryColor);
    }
    return std::string();
  }
  return ctorIt->second(secondaryColor);
}

std::string OperationIcons::createIcon(const Index& index, const std::string& secondaryColor) const
{
  auto ctorIt = m_functors.find(index);
  if (ctorIt == m_functors.end())
  {
    if (m_defaultIconConstructor)
    {
      return m_defaultIconConstructor(secondaryColor);
    }
    return std::string();
  }
  return ctorIt->second(secondaryColor);
}
} // namespace view
} // namespace smtk
