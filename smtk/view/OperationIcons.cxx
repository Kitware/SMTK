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

using namespace smtk::string::literals;

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
  return this->createIcon(operationName, secondaryColor, "normal"_token);
}

std::string OperationIcons::createIcon(
  const std::string& operationName,
  const std::string& secondaryColor,
  smtk::string::Token mode) const
{
  auto nameIt = m_indices.find(operationName);
  FunctorMap::const_iterator ctorIt;
  FunctorMap2::const_iterator ctorIt2;
  if (nameIt != m_indices.end())
  {
    ctorIt = m_functors.find(nameIt->second);
    if (ctorIt != m_functors.end())
    {
      return ctorIt->second(secondaryColor);
    }
    ctorIt2 = m_functors2.find(nameIt->second);
    if (ctorIt2 != m_functors2.end())
    {
      return ctorIt2->second(secondaryColor, mode);
    }
  }
  if (m_defaultIconConstructor)
  {
    return m_defaultIconConstructor(secondaryColor);
  }
  return std::string();
}

std::string OperationIcons::createIcon(const Index& index, const std::string& secondaryColor) const
{
  return this->createIcon(index, secondaryColor, "normal"_token);
}

std::string OperationIcons::createIcon(
  const Index& index,
  const std::string& secondaryColor,
  smtk::string::Token mode) const
{
  auto ctorIt2 = m_functors2.find(index);
  if (ctorIt2 != m_functors2.end())
  {
    return ctorIt2->second(secondaryColor, mode);
  }
  auto ctorIt = m_functors.find(index);
  if (ctorIt != m_functors.end())
  {
    return ctorIt->second(secondaryColor);
  }
  if (m_defaultIconConstructor)
  {
    return m_defaultIconConstructor(secondaryColor);
  }
  return std::string();
}

} // namespace view
} // namespace smtk
