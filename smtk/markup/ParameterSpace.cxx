//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/ParameterSpace.h"

#include "smtk/markup/DiscreteGeometry.h"

namespace smtk
{
namespace markup
{

ParameterSpace::ParameterSpace(smtk::string::Token name)
  : Domain(name)
{
}

ParameterSpace::ParameterSpace(const nlohmann::json& data)
  : Domain(data)
{
  // TODO: deserialize m_data
}

bool ParameterSpace::setData(const std::weak_ptr<smtk::markup::DiscreteGeometry>& data)
{
  auto mlocked = m_data.lock();
  auto vlocked = data.lock();
  if (mlocked == vlocked)
  {
    return false;
  }
  m_data = vlocked;
  return true;
}

const std::weak_ptr<smtk::markup::DiscreteGeometry>& ParameterSpace::data() const
{
  return m_data;
}

std::weak_ptr<smtk::markup::DiscreteGeometry>& ParameterSpace::data()
{
  return m_data;
}

} // namespace markup
} // namespace smtk
