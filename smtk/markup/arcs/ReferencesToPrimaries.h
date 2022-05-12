//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_arcs_ReferencesToPrimaries_h
#define smtk_markup_arcs_ReferencesToPrimaries_h
/*!\file */

#include "smtk/markup/Exports.h"

#include "smtk/graph/OwnershipSemantics.h"

#include "smtk/common/Visit.h"

#include <cstddef>
#include <type_traits>

namespace smtk
{
namespace markup
{

class SpatialData;

namespace arcs
{

/**\brief Arcs connecting spatial data to the domains over which they are defined.
  *
  * These arcs are implicit since we can infer them from the ID assignments
  * held by DiscreteGeometry. Currently, we do not implement ParameterSpace
  * domains; once we do, similar arcs will exist for parametric shapes.
  */
struct SMTKMARKUP_EXPORT ReferencesToPrimaries
{
  using FromType = SpatialData;
  using ToType = SpatialData;
  using Directed = std::true_type;

  /// Are the \a from and \to nodes connected?
  ///
  /// When a domain and shape (spatial data object) are connected,
  /// the \a from domain mathematically covers the \a to shape (i.e.,
  /// the shape is a subset of the domain).
  bool contains(const SpatialData* from, const SpatialData* to) const;

  /// Return the number of shapes participating in the domain.
  std::size_t outDegree(const SpatialData* from) const;
  /// Return the number domains this shape participates in.
  std::size_t inDegree(const SpatialData* to) const;

  /// Visit each shape participating in the \a domain.
  template<typename Functor>
  smtk::common::Visited outVisitor(const SpatialData* domain, Functor ff) const;

  /// Visit each domain this \a shape participates in.
  template<typename Functor>
  smtk::common::Visited inVisitor(const SpatialData* shape, Functor ff) const;

  /// Visit all domains in the resource which have arcs to one or more shapes.
  template<typename ResourcePtr, typename Functor>
  smtk::common::Visited visitAllOutgoingNodes(ResourcePtr rr, Functor ff) const;

  /// Visit all shapes in the resource which have arcs to one or more domains.
  template<typename ResourcePtr, typename Functor>
  smtk::common::Visited visitAllIncomingNodes(ResourcePtr rr, Functor ff) const;
};

} // namespace arcs
} // namespace markup
} // namespace smtk

#endif // smtk_markup_arcs_ReferencesToPrimaries_h
