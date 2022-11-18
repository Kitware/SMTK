//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/Box.h"

namespace smtk
{
namespace markup
{

void Box::initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper)
{
  (void)helper;
  m_range[0] = data.at("lo").get<std::array<double, 3>>();
  m_range[1] = data.at("hi").get<std::array<double, 3>>();
}

void Box::initialize(const std::array<double, 3>& lo, const std::array<double, 3>& hi)
{
  m_range[0] = lo;
  m_range[1] = hi;
}

bool Box::setRange(const std::array<std::array<double, 3>, 2>& range)
{
  if (m_range == range)
  {
    return false;
  }
  m_range = range;
  return true;
}

const std::array<std::array<double, 3>, 2>& Box::range() const
{
  return m_range;
}

std::array<std::array<double, 3>, 2>& Box::range()
{
  return m_range;
}

} // namespace markup
} // namespace smtk
