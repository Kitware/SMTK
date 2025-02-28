//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_query_Factory_h
#define smtk_resource_query_Factory_h

#include "smtk/CoreExports.h"
#include "smtk/TupleTraits.h"

#include "smtk/common/TypeName.h"

#include "smtk/resource/query/BadTypeError.h"
#include "smtk/resource/query/Metadata.h"
#include "smtk/resource/query/Query.h"

#include <memory>
#include <type_traits>
#include <unordered_set>

namespace smtk
{
namespace resource
{
namespace query
{

/// A factory for registering and creating Query instances. Query types are
/// selected according to a priority functor that is registered along with the
/// Query type. The default priority functor favors Query types that are most
/// derived from the input Query type.
class SMTKCORE_EXPORT Factory
{
public:
  /// Register a Query type using the default priority functor. Queries must be
  /// default constructible.
  template<typename QueryType>
  bool registerQuery()
  {
    return registerQuery(Metadata(identity<QueryType>()));
  }

  /// Register a Query type using a provided priority functor. Queries must be
  /// default constructible.
  template<typename QueryType>
  bool registerQuery(std::function<int(const std::size_t&)> priority)
  {
    return registerQuery(Metadata(identity<QueryType>(), priority));
  }

  /// Register a Query type using the Query type's Metadata.
  bool registerQuery(Metadata&& metadata);

  /// Register a tuple of Query types using the default priority functor.
  /// Queries must be default constructibel.
  template<typename Tuple>
  bool registerQueries()
  {
    return registerQueries<0, Tuple>();
  }

  /// Unregister a Query type.
  template<typename QueryType>
  bool unregisterQuery()
  {
    return unregisterQuery(QueryType::typeIndex());
  }

  /// Unregister a Query type using its type index.
  bool unregisterQuery(std::size_t typeIndex);

  /// Unregister a tuple of Query types.
  template<typename Tuple>
  bool unregisterQueries()
  {
    return unregisterQueries<0, Tuple>();
  }

  /// Determine whether or not a Query type (or a suitable replacement for it)
  /// is available.
  template<typename QueryType>
  bool contains() const
  {
    return contains(QueryType::typeIndex());
  }

  /// Determine whether or not a Query type (or a suitable replacement for it)
  /// is available using its type index.
  bool contains(std::size_t typeIndex) const;

  /// Create an instance of a concrete Query type (or a suitable replacement for
  /// it), registering the Query type if it is not already registered.
  template<typename QueryType>
  typename std::enable_if<!std::is_abstract<QueryType>::value, std::unique_ptr<QueryType>>::type
  create()
  {
    if (!contains<QueryType>())
    {
      registerQuery<QueryType>();
    }

    return std::unique_ptr<QueryType>{ static_cast<QueryType*>(
      create(QueryType::typeIndex()).release()) };
  }

  /// Create a a suitable replacement for an abstract Query type.
  template<typename QueryType>
  typename std::enable_if<std::is_abstract<QueryType>::value, std::unique_ptr<QueryType>>::type
  create()
  {
    return std::unique_ptr<QueryType>{ static_cast<QueryType*>(
      create(QueryType::typeIndex()).release()) };
  }

  /// Create an instance of a Query type (or a suitable replacement for it).
  std::unique_ptr<Query> create(const std::size_t& typeIndex) const;

  template<typename QueryType>
  std::size_t indexFor() const
  {
    return indexFor(QueryType::typeIndex());
  }

  /// Given a concrete input Query type, return the type index of a registered
  /// Query type to perform the query, adding it if necessary.
  template<typename QueryType>
  typename std::enable_if<!std::is_abstract<QueryType>::value, std::size_t>::type indexFor()
  {
    if (!contains<QueryType>())
    {
      registerQuery<QueryType>();
    }

    return indexFor(QueryType::typeIndex());
  }

  /// Given an abstract input Query type, return the type index of a registered
  /// Query type to perform the query.
  template<typename QueryType>
  typename std::enable_if<std::is_abstract<QueryType>::value, std::size_t>::type indexFor()
  {
    return indexFor(QueryType::typeIndex());
  }

  /// Given an input Query type (which may be abstract), return the type index
  /// of a registered Query type to perform the query.
  std::size_t indexFor(const std::size_t& typeIndex) const;

private:
  template<std::size_t I, typename Tuple>
  typename std::enable_if<I != std::tuple_size<Tuple>::value, bool>::type registerQueries()
  {
    bool registered = this->registerQuery<typename std::tuple_element<I, Tuple>::type>();
    return registered && registerQueries<I + 1, Tuple>();
  }

  template<std::size_t I, typename Tuple>
  typename std::enable_if<I == std::tuple_size<Tuple>::value, bool>::type registerQueries()
  {
    return true;
  }

  template<std::size_t I, typename Tuple>
  typename std::enable_if<I != std::tuple_size<Tuple>::value, bool>::type unregisterQueries()
  {
    bool unregistered = this->unregisterQuery<typename std::tuple_element<I, Tuple>::type>();
    return unregistered && unregisterQueries<I + 1, Tuple>();
  }

  template<std::size_t I, typename Tuple>
  typename std::enable_if<I == std::tuple_size<Tuple>::value, bool>::type unregisterQueries()
  {
    return true;
  }

  struct HashByTypeIndex
  {
    std::size_t operator()(const Metadata& metadata) const { return metadata.index(); }
  };
  struct EquateByTypeIndex
  {
    bool operator()(const Metadata& lhs, const Metadata& rhs) const
    {
      return lhs.index() == rhs.index();
    }
  };
  std::unordered_set<Metadata, HashByTypeIndex, EquateByTypeIndex> m_metadata;
};
} // namespace query
} // namespace resource
} // namespace smtk

#endif
