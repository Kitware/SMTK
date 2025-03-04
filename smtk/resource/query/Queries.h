//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_resource_query_Queries_h
#define smtk_resource_query_Queries_h

#include "smtk/CoreExports.h"

#include "smtk/resource/query/Cache.h"
#include "smtk/resource/query/Container.h"
#include "smtk/resource/query/Factory.h"

#include <utility>

namespace smtk
{
namespace resource
{
namespace query
{
/// A class for registering and accessing Query types. This class is a Factory
/// with additional storage for cached Query types and for Query Caches.
class SMTKCORE_EXPORT Queries
{
public:
  Queries() = default;
  virtual ~Queries() = default;

  Queries(const Queries&) = delete;
  Queries(Queries&& rhs) noexcept
    : m_caches(std::move(rhs.m_caches))
    , m_queries(std::move(rhs.m_queries))
    , m_factory(std::move(rhs.m_factory))
  {
  }
  Queries& operator=(const Queries&) = delete;
  Queries& operator=(Queries&& rhs) noexcept
  {
    m_caches = std::move(rhs.m_caches);
    m_queries = std::move(rhs.m_queries);
    m_factory = std::move(rhs.m_factory);
    return *this;
  }

  /// Register a Query type using the default priority functor. Queries must be
  /// default constructible.
  template<typename QueryType>
  bool registerQuery()
  {
    return m_factory.registerQuery<QueryType>();
  }

  /// Register a Query type using a provided priority functor. Queries must be
  /// default constructible.
  template<typename QueryType>
  bool registerQuery(std::function<int(const std::size_t&)>&& priority)
  {
    return m_factory.registerQuery<QueryType>(
      std::forward<std::function<int(const std::size_t&)>>(priority));
  }

  /// Register a tuple of Query types using the default priority functor.
  /// Queries must be default constructibel.
  template<typename Tuple>
  bool registerQueries()
  {
    return m_factory.registerQueries<Tuple>();
  }

  /// Unregister a Query type.
  template<typename QueryType>
  bool unregisterQuery()
  {
    return m_factory.unregisterQuery<QueryType>();
  }

  /// Unregister a tuple of Query types.
  template<typename Tuple>
  bool unregisterQueries()
  {
    return m_factory.unregisterQueries<Tuple>();
  }

  /// Determine whether or not a Query type (or a suitable replacement for it)
  /// is available.
  template<typename QueryType>
  bool contains() const
  {
    return m_factory.contains<QueryType>();
  }

  /// Access a Query type, constructing one if necessary.
  template<typename QueryType>
  QueryType& get() const
  {
    std::size_t index = m_factory.indexFor<QueryType>();

    // Attempt to access an existing query object.
    try
    {
      return static_cast<QueryType&>(m_queries.get(index));
    }
    catch (BadTypeError&)
    {
      // If none is found, attempt to create a new one.
      auto& queries = m_queries.data();
      auto query = m_factory.create(index);
      // If one cannot be created, rethrow.
      if (!query)
      {
        throw;
      }
      auto inserted = queries.emplace(index, std::move(query)).first;
      return static_cast<QueryType&>(*(inserted->second));
    }
  }

  /// Access a Query Cache.
  template<typename CacheType>
  CacheType& cache() const
  {
    return m_caches.get<CacheType>();
  }

  decltype(std::declval<Container<Cache>>().data()) caches() { return m_caches.data(); }
  decltype(std::declval<Container<Query>>().data()) data() { return m_queries.data(); }
  Factory& factory() { return m_factory; }

private:
  mutable Container<Cache> m_caches;
  mutable Container<Query> m_queries;
  Factory m_factory;
};
} // namespace query
} // namespace resource
} // namespace smtk

#endif
