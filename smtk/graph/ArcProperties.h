//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_graph_ArcProperties_h
#define smtk_graph_ArcProperties_h

#include "smtk/Metaprogramming.h"
#include "smtk/common/Visit.h"
#include "smtk/graph/OwnershipSemantics.h"
#include "smtk/string/Token.h"

#include <functional>
#include <iterator>
#include <set>
#include <type_traits>

namespace smtk
{
namespace graph
{

/// Return a constant used to indicate the maximimum degree of an arc endpoint is unconstrained.
constexpr std::size_t unconstrained()
{
  return std::numeric_limits<std::size_t>::max();
}

/**\brief Checks that can be performed on arc trait-types.
  *
  * Note that the properties defined in this class are named like so:
  * + direct checks on the \a ArcTraits template parameter are named `hasXXX`.
  * + results that specify behavior of the resulting arc implementation are named `isXXX`.
  * (i.e., "If the traits have this, then the arc is that.")
  */
template<typename ArcTraits>
class ArcProperties
{
  template<class>
  struct type_sink
  {
    using type = void;
  }; // consume a type and make it `void`
  template<class T>
  using type_sink_t = typename type_sink<T>::type;

public:
  /// Check that a method exists to visit out-nodes given a "from" node.
  template<class T, class = void>
  struct hasOutVisitor : std::false_type
  {
  };
  template<class T>
  struct hasOutVisitor<
    T,
    type_sink_t<decltype(std::declval<T>().outVisitor(
      nullptr,
      std::function<smtk::common::Visited(typename T::ToType const*)>()))>> : std::true_type
  {
  };

  /// Check that a method exists to visit in-nodes given a "to" node.
  template<class T, class = void>
  struct hasInVisitor : std::false_type
  {
  };
  template<class T>
  struct hasInVisitor<
    T,
    type_sink_t<decltype(std::declval<T>().inVisitor(
      nullptr,
      std::function<smtk::common::Visited(typename T::FromType const*)>()))>> : std::true_type
  {
  };

  /// Check that a method exists to visit all nodes with outgoing arcs.
  template<class T, class = void>
  struct hasOutNodeVisitor : std::false_type
  {
  };
  template<class T>
  struct hasOutNodeVisitor<
    T,
    type_sink_t<decltype(std::declval<T>().visitAllOutgoingNodes(
      nullptr,
      std::function<smtk::common::Visited(typename T::FromType const*)>()))>> : std::true_type
  {
  };

  /// Check that a method exists to visit all nodes with incoming arcs.
  template<class T, class = void>
  struct hasInNodeVisitor : std::false_type
  {
  };
  template<class T>
  struct hasInNodeVisitor<
    T,
    type_sink_t<decltype(std::declval<T>().visitAllIncomingNodes(
      nullptr,
      std::function<smtk::common::Visited(typename T::ToType const*)>()))>> : std::true_type
  {
  };

  /// Check that a method exists to test existence of an arc.
  template<class T, class = void>
  struct hasContains : std::false_type
  {
  };
  template<class T>
  struct hasContains<
    T,
    type_sink_t<decltype(std::declval<T>().contains(
      static_cast<const typename ArcTraits::FromType*>(nullptr),
      static_cast<const typename ArcTraits::ToType*>(nullptr)))>> : std::true_type
  {
  };

  /// Check that a method exists to compute the out-degree of an arc.
  template<class T, class = void>
  struct hasOutDegree : std::false_type
  {
  };
  template<class T>
  struct hasOutDegree<
    T,
    type_sink_t<decltype(std::declval<T>().outDegree(
      static_cast<const typename ArcTraits::FromType*>(nullptr)))>> : std::true_type
  {
  };

  /// Check that a method exists to compute the in-degree of an arc.
  template<class T, class = void>
  struct hasInDegree : std::false_type
  {
  };
  template<class T>
  struct hasInDegree<
    T,
    type_sink_t<decltype(std::declval<T>().inDegree(
      static_cast<const typename ArcTraits::ToType*>(nullptr)))>> : std::true_type
  {
  };

  /// Check that a method exists to insert arcs.
  template<class T, class = void>
  struct hasConnect : std::false_type
  {
  };
  template<class T>
  struct hasConnect<
    T,
    type_sink_t<decltype(std::declval<T>().connect(
      static_cast<typename ArcTraits::FromType*>(nullptr),
      static_cast<typename ArcTraits::ToType*>(nullptr),
      static_cast<typename ArcTraits::FromType*>(nullptr),
      static_cast<typename ArcTraits::ToType*>(nullptr)))>> : std::true_type
  {
  };

  /// Check that a method exists to remove arcs.
  template<class T, class = void>
  struct hasDisconnect : std::false_type
  {
  };
  template<class T>
  struct hasDisconnect<
    T,
    type_sink_t<decltype(std::declval<T>().disconnect(
      static_cast<typename ArcTraits::FromType*>(nullptr),
      static_cast<typename ArcTraits::ToType*>(nullptr)))>> : std::true_type
  {
  };

  /**\brief Check whether the arc has bidirectional indexing (the default)
    *       or has been marked as having only a forward index.
    */
  template<class T, class = void>
  struct hasOnlyForwardIndex : std::false_type
  {
  };
  template<class T>
  struct hasOnlyForwardIndex<T, type_sink_t<typename T::ForwardIndexOnly>>
    : std::conditional<T::ForwardIndexOnly::value, std::true_type, std::false_type>::type
  {
  };

  /**\brief Check whether the traits object has been marked immutable.
    */
  template<class T, class = void>
  struct hasImmutableMark : std::false_type
  {
  };
  template<class T>
  struct hasImmutableMark<T, type_sink_t<typename T::Immutable>> : std::true_type
  {
    // Complain if we have ForwardIndexOnly but it is not true.
    static_assert(T::Immutable::value, "Immutable must be true_type if present.");
  };

  /// True when the order in which arcs are stored is significant.
  class isOrdered
  {
  public:
    using type = typename hasOnlyForwardIndex<ArcTraits>::type;
    static constexpr bool value = type::value;
  };

  /**\brief Check whether the arc traits object allows traversal only in the foward direction.
    *
    * Either (1) the arc traits has an in-visitor but explicitly disallows its use or
    * (2) the arc traits doesn't have an in-visitor method.
    */
  class isOnlyForwardIndexed
  {
  public:
    using type = typename disjunction<
      // We have an inVisitor but want to disable it (using OnlyForwardIndex)
      conjunction<hasOnlyForwardIndex<ArcTraits>, hasInVisitor<ArcTraits>>,
      // We do not have an inVisitor but do have an outVisitor
      conjunction<negation<hasInVisitor<ArcTraits>>, hasOutVisitor<ArcTraits>>>::type;
    static constexpr bool value = type::value;
  };

  /// True when an arc class provides implementations of required methods; false otherwise.
  class isImplicit
  {
  public:
    using type = typename conjunction<
      // We must have an outVisitor() method.
      hasOutVisitor<ArcTraits>,
      // We must have an inVisitor() method or be marked as only allowing forward traversal.
      disjunction<isOnlyForwardIndexed, hasInVisitor<ArcTraits>>>::type;
    static constexpr bool value = type::value;
  };

  /// False when an arc class provides implementations of required methods; true otherwise.
  class isExplicit
  {
  public:
    using type = typename negation<isImplicit>::type;
    static constexpr bool value = type::value;
  };

  /**\brief Check whether the arc traits object
    *       (1) is implicit and has methods to insert and remove arcs; or
    *       (2) is explicit and has not been marked immutable.
    */
  class isMutable
  {
  public:
    using type = typename conjunction<
      negation<hasImmutableMark<ArcTraits>>,
      disjunction<
        conjunction<isImplicit, hasConnect<ArcTraits>, hasDisconnect<ArcTraits>>,
        isExplicit>>::type;
    static constexpr bool value = type::value;
  };

  /**\brief Check whether the traits object is undirected and has identical to/from types.
    *
    * In this case, many methods must behave differently since an arc
    * between nodes a and b may be stored as either "a→b" or "b→a"
    * (since the graph is undirected, the order is immaterial).
    * We call this property "isAutoUndirected" since only nodes that
    * point to other nodes of the same type (hence "auto") can truly
    * be undirected as far as the API is concerned.
    */
  class isAutoUndirected
  {
  public:
    using type = typename conjunction<
      std::is_same<typename ArcTraits::FromType, typename ArcTraits::ToType>,
      negation<typename ArcTraits::Directed>>::type;
    static constexpr bool value = type::value;
  };

protected:
  template<class T, class = void>
  struct hasSemantics : std::false_type
  {
    constexpr OwnershipSemantics operator()() const { return OwnershipSemantics::None; }
  };
  template<class T>
  struct hasSemantics<T, type_sink_t<decltype(T::semantics)>> : std::true_type
  {
    constexpr OwnershipSemantics operator()() const { return T::semantics; }
  };

public:
  /**\brief Check whether the traits object provides ownership semantics and return them.
    *
    * Ownership semantics indicate whether one arc endpoint should keep the other
    * endpoint from being removed from the resource unless both are removed.
    * If an ArcTraits type provides no semantics, then this returns OwnershipSemantics::None.
    *
    * For example, an arc connecting a Group node to its Member nodes might be
    * marked ToNodeOwnsFromNode, indicating that the Group node (i.e., ArcTraits::FromType)
    * should not be removed without also removing all of its Member nodes (ArcTraits::ToType).
    * However, any Member node could be removed without a problem (assuming no other owning arcs
    * exist).
    *
    * Similarly, if the arc was marked with FromNodeOwnsToNode, then Member nodes could not
    * be deleted unless the Group node was also deleted; but the Group node could be
    * deleted even when it has arcs to Member nodes.
    */
  static constexpr OwnershipSemantics ownershipSemantics() { return hasSemantics<ArcTraits>()(); }

  // clang-format off
  /// Test whether an arc-traits object provides an "accepts()"
  /// method with the same signature as "connect()". If present,
  /// this method will return whether an arc between two nodes
  /// will be allowed.
  ///@{
  template<class T, class = void>
  struct hasAccepts : std::false_type
  {
  };
  template<class T>
  struct hasAccepts<
    T,
    type_sink_t<
      decltype(std::declval<T>().accepts(nullptr, nullptr, nullptr, nullptr))>> : std::true_type
  {
  };
  ///@}

  /// Test whether an arc-traits object provides a "size()" method
  /// that returns the number of arcs of its type in the entire graph.
  ///@{
  template<class T, class = void>
  struct hasSize : std::false_type
  {
  };
  template<class T>
  struct hasSize<
    T,
    type_sink_t<
      decltype(std::declval<T>().size())>> : std::true_type
  {
  };
  ///@}
  // clang-format on

  /// Does the traits object have a member named m_fromNodeSpecs?
  template<typename U = ArcTraits, typename = void>
  struct hasFromNodeSpecs : std::false_type
  {
  };
  template<typename U>
  struct hasFromNodeSpecs<U, type_sink_t<decltype(std::declval<U>().m_fromNodeSpecs)>>
    : std::true_type
  {
  };

  /// Does the traits object have a member named m_toNodeSpecs?
  template<typename U = ArcTraits, typename = void>
  struct hasToNodeSpecs : std::false_type
  {
  };
  template<typename U>
  struct hasToNodeSpecs<U, type_sink_t<decltype(std::declval<U>().m_toNodeSpecs)>> : std::true_type
  {
  };
};

/// Return the maximum out-degree of an arc type (or unconstrained() if unspecified).
//@{
template<typename T>
constexpr
  typename std::enable_if<std::is_integral<decltype(T::MaxOutDegree)>::value, std::size_t>::type
  maxOutDegree(std::size_t)
{
  return T::MaxOutDegree;
}

template<typename T>
constexpr std::size_t maxOutDegree(...)
{
  return smtk::graph::unconstrained();
}
//@}

/// Return the maximum in-degree of an arc type (or unconstrained() if unspecified).
//@{
template<typename T>
constexpr
  typename std::enable_if<std::is_integral<decltype(T::MaxInDegree)>::value, std::size_t>::type
  maxInDegree(std::size_t)
{
  return T::MaxInDegree;
}

template<typename T>
constexpr std::size_t maxInDegree(...)
{
  return smtk::graph::unconstrained();
}
//@}

/// Return the minimum out-degree of an arc type (or 0 if unspecified).
//@{
template<typename T>
constexpr
  typename std::enable_if<std::is_integral<decltype(T::MinOutDegree)>::value, std::size_t>::type
  minOutDegree(std::size_t)
{
  return T::MinOutDegree;
}

template<typename T>
constexpr std::size_t minOutDegree(...)
{
  return 0;
}
//@}

/// Return the minimum in-degree of an arc type (or 0 if unspecified).
//@{
template<typename T>
constexpr
  typename std::enable_if<std::is_integral<decltype(T::MinInDegree)>::value, std::size_t>::type
  minInDegree(std::size_t)
{
  return T::MinInDegree;
}

template<typename T>
constexpr std::size_t minInDegree(...)
{
  return 0;
}
//@}

} // namespace graph
} // namespace smtk

#endif
