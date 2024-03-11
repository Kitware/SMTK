//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/Plane.h"

namespace smtk
{
namespace markup
{

Plane::~Plane() = default;

void Plane::initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper)
{
  (void)data;
  (void)helper;
}

bool Plane::setBasePoint(const std::array<double, 3>& basePoint)
{
  if (m_basePoint == basePoint)
  {
    return false;
  }
  m_basePoint = basePoint;
  return true;
}

const std::array<double, 3>& Plane::basePoint() const
{
  return m_basePoint;
}

std::array<double, 3>& Plane::basePoint()
{
  return m_basePoint;
}

bool Plane::setNormal(const std::array<double, 3>& normal)
{
  if (m_normal == normal)
  {
    return false;
  }
  m_normal = normal;
  return true;
}

const std::array<double, 3>& Plane::normal() const
{
  return m_normal;
}

std::array<double, 3>& Plane::normal()
{
  return m_normal;
}

bool Plane::assign(
  const smtk::graph::Component::ConstPtr& source,
  smtk::resource::CopyOptions& options)
{
  bool ok = this->Superclass::assign(source, options);
  if (auto sourcePlane = std::dynamic_pointer_cast<const Plane>(source))
  {
    this->setBasePoint(sourcePlane->basePoint());
    this->setNormal(sourcePlane->normal());
  }
  else
  {
    ok = false;
  }
  return ok;
}

} // namespace markup
} // namespace smtk
