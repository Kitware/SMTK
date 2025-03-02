//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_resource_query_DerivedFrom_h
#define smtk_resource_query_DerivedFrom_h

#include "smtk/CoreExports.h"

#include "smtk/resource/query/Query.h"

#include <typeindex>

namespace smtk
{
namespace resource
{
namespace query
{

/// Query functors can be registered to Resources as the implementation of a
/// base (and often abstract) Query. The default priority functor for a Query
/// type is related to its hierarchical relationship to the base Query type (the
/// most derived registered class is returned); a different priority functor can
/// be assigned at registration time. CRTP is used to ensure that Query classes
/// all have an associated type_index and to facilitate type walking for the
/// default priority algorithm.
template<typename SelfType, typename ParentType>
class SMTK_ALWAYS_EXPORT DerivedFrom : public ParentType
{
  static_assert(
    std::is_base_of<Query, ParentType>::value,
    "Queries must inherit from smtk::resource::query::Query or its children");

  friend class Metadata;

public:
  static std::size_t typeIndex() { return typeid(SelfType).hash_code(); }

protected:
  typedef ParentType Parent;

  /// Return the number of generations from this instance to the type index of
  /// a parent, or return a large negative number if the two types are
  /// unrelated.
  static int numberOfGenerationsFromType(std::size_t index)
  {
    return (
      DerivedFrom<SelfType, Parent>::typeIndex() == index
        ? 0
        : 1 + Parent::numberOfGenerationsFromType(index));
  }
};
} // namespace query
} // namespace resource
} // namespace smtk

#endif
