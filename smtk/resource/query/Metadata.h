//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_query_Metadata_h
#define smtk_resource_query_Metadata_h

#include "smtk/CoreExports.h"

#include "smtk/resource/query/Query.h"

#include <functional>
#include <typeindex>

namespace smtk
{
namespace resource
{
namespace query
{

/// A structure to hold type and construction information about a Query type.
class SMTKCORE_EXPORT Metadata
{
  friend class Factory;

  Metadata(
    std::size_t index,
    std::function<int(const std::size_t&)> priorityFunctor,
    std::function<Query*()> createFunctor)
    : create(createFunctor)
    , priority(priorityFunctor)
    , m_index(index)
  {
  }

  template<typename QueryType>
  Metadata(identity<QueryType>, std::function<int(const std::size_t&)> priorityFunctor)
    : Metadata(QueryType::typeIndex(), priorityFunctor, []() { return new QueryType; })
  {
  }

  template<typename QueryType>
  Metadata(identity<QueryType>)
    : Metadata(
        QueryType::typeIndex(),
        [](const std::size_t& typeIndex) {
          return QueryType::numberOfGenerationsFromType(typeIndex);
        },
        []() { return new QueryType; })
  {
  }

  const std::size_t& index() const { return m_index; }

  std::function<Query*()> create = []() { return nullptr; };
  std::function<int(const std::size_t&)> priority = [](const std::size_t&) {
    return std::numeric_limits<int>::lowest();
  };

  static Metadata key(const std::size_t& index) { return Metadata(index); }

  Metadata(const std::size_t& index)
    : m_index(index)
  {
  }

  std::size_t m_index;
};
} // namespace query
} // namespace resource
} // namespace smtk

#endif
