//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/markup/DomainMap.h"

#include "smtk/markup/Domain.h"

namespace smtk
{
namespace markup
{

bool DomainMap::contains(const smtk::string::Token& space) const
{
  auto it = m_domains.find(space);
  bool have = (it == m_domains.end());
  return have;
}

bool DomainMap::insert(const smtk::string::Token& space, const std::shared_ptr<Domain>& domain)
{
  auto it = m_domains.find(space);
  if (it != m_domains.end())
  {
    return false;
  }

  m_domains[space] = domain;
  return true;
}

std::shared_ptr<Domain> DomainMap::find(const smtk::string::Token& space) const
{
  auto it = m_domains.find(space);
  if (it == m_domains.end())
  {
    return nullptr;
  }
  return it->second;
}

std::shared_ptr<Domain> DomainMap::operator[](const smtk::string::Token& space) const
{
  return this->find(space);
}

std::unordered_set<smtk::string::Token> DomainMap::keys() const
{
  std::unordered_set<smtk::string::Token> result;
  for (const auto& entry : m_domains)
  {
    result.insert(entry.first);
  }
  return result;
}

} // namespace markup
} // namespace smtk
