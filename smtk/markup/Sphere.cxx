//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/Sphere.h"

namespace smtk
{
namespace markup
{

void Sphere::initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper)
{
  (void)helper;
  m_center = data.at("center").get<std::array<double, 3>>();
  m_radius = data.at("radius").get<std::array<double, 3>>();
}

void Sphere::initialize(const std::array<double, 3>& center, double radius)
{
  m_center = center;
  m_radius = { radius, radius, radius };
}

void Sphere::initialize(const std::array<double, 3>& center, const std::array<double, 3>& radii)
{
  m_center = center;
  m_radius = radii;
}

bool Sphere::setCenter(const std::array<double, 3>& center)
{
  if (m_center == center)
  {
    return false;
  }
  m_center = center;
  return true;
}

const std::array<double, 3>& Sphere::center() const
{
  return m_center;
}

std::array<double, 3>& Sphere::center()
{
  return m_center;
}

bool Sphere::setRadius(const std::array<double, 3>& radius)
{
  if (m_radius == radius)
  {
    return false;
  }
  m_radius = radius;
  return true;
}

const std::array<double, 3>& Sphere::radius() const
{
  return m_radius;
}

std::array<double, 3>& Sphere::radius()
{
  return m_radius;
}

bool Sphere::assign(
  const smtk::graph::Component::ConstPtr& source,
  smtk::resource::CopyOptions& options)
{
  bool ok = this->Superclass::assign(source, options);
  if (auto sourceSphere = std::dynamic_pointer_cast<const Sphere>(source))
  {
    this->setCenter(sourceSphere->center());
    this->setRadius(sourceSphere->radius());
  }
  else
  {
    ok = false;
  }
  return ok;
}

} // namespace markup
} // namespace smtk
