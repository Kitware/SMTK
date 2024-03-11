//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/Cone.h"

namespace smtk
{
namespace markup
{

Cone::~Cone() = default;

void Cone::initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper)
{
  (void)data;
  (void)helper;
}

bool Cone::setEndpoints(const std::array<std::array<double, 3>, 2>& endpoints)
{
  if (m_endpoints == endpoints)
  {
    return false;
  }
  m_endpoints = endpoints;
  return true;
}

const std::array<std::array<double, 3>, 2>& Cone::endpoints() const
{
  return m_endpoints;
}

std::array<std::array<double, 3>, 2>& Cone::endpoints()
{
  return m_endpoints;
}

bool Cone::setRadii(const std::array<double, 2>& radii)
{
  if (m_radii == radii)
  {
    return false;
  }
  m_radii = radii;
  return true;
}

const std::array<double, 2>& Cone::radii() const
{
  return m_radii;
}

std::array<double, 2>& Cone::radii()
{
  return m_radii;
}

bool Cone::assign(
  const smtk::graph::Component::ConstPtr& source,
  smtk::resource::CopyOptions& options)
{
  bool ok = this->Superclass::assign(source, options);
  if (auto sourceCone = std::dynamic_pointer_cast<const Cone>(source))
  {
    m_endpoints = sourceCone->m_endpoints;
    m_radii = sourceCone->m_radii;
  }
  else
  {
    ok = false;
  }
  return ok;
}

} // namespace markup
} // namespace smtk
