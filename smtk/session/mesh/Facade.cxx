//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#include "smtk/session/mesh/Facade.h"

#include <map>

namespace smtk
{
namespace session
{
namespace mesh
{

struct Facade::Internals
{
  std::map<std::string, std::string> m_data;
};

Facade::Facade()
  : m_internal(new Internals)
{

  m_internal->m_data["domain"] = "Domain";
  m_internal->m_data["dirichlet"] = "Dirichlet";
  m_internal->m_data["neumann"] = "Neumann";
}

Facade::~Facade()
{
  delete m_internal;
}

std::string& Facade::operator[](const std::string& key)
{
  static std::string dummy;
  dummy = "";
  auto loc = m_internal->m_data.find(key);
  return loc == m_internal->m_data.end() ? dummy : loc->second;
}
} // namespace mesh
} // namespace session
} // namespace smtk
