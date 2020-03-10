//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/resource/query/Factory.h"

namespace smtk
{
namespace resource
{
namespace query
{

bool Factory::registerQuery(Metadata&& metadata)
{
  return m_metadata.emplace(std::forward<Metadata>(metadata)).second;
}

bool Factory::unregisterQuery(const std::size_t typeIndex)
{
  auto search = m_metadata.find(Metadata::key(typeIndex));
  if (search != m_metadata.end())
  {
    return m_metadata.erase(*search) > 0;
  }
  return false;
}

bool Factory::contains(const std::size_t typeIndex) const
{
  auto search = m_metadata.find(Metadata::key(indexFor(typeIndex)));
  return search != m_metadata.end();
}

std::unique_ptr<Query> Factory::create(const std::size_t& typeIndex) const
{
  std::size_t index = indexFor(typeIndex);
  return (index != 0 ? std::unique_ptr<Query>(m_metadata.find(Metadata::key(index))->create())
                     : std::unique_ptr<Query>());
}

std::size_t Factory::indexFor(const std::size_t& typeIndex) const
{
  int priority = std::numeric_limits<int>::lowest();
  const Metadata* metadata = nullptr;
  for (auto& metadatum : m_metadata)
  {
    int n = metadatum.priority(typeIndex);
    if (n > priority)
    {
      priority = n;
      metadata = &metadatum;
    }
  }

  if (metadata != nullptr)
  {
    return metadata->index();
  }
  return 0;
}
}
}
}
