//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_graph_TypeTraits_h
#define smtk_graph_TypeTraits_h

#include "smtk/common/TypeTraits.h"
#include "smtk/common/Visit.h"

#include "smtk/graph/ArcProperties.h"
#include "smtk/graph/ExplicitArcs.h"

#include <functional>
#include <iterator>
#include <set>
#include <type_traits>

namespace smtk
{
namespace graph
{

class NodeSet;

namespace detail
{

/// Provide no default arc storage.
template<typename ArcTraits, typename Storage>
struct SelectArcContainer;

/**\brief Specialize explicit arc storage when unordered.
  *
  * Unless \a ArcTraits provides methods for accessing
  * (and if, editable, manipulating) arcs – the graph resource will
  * explicitly store arcs in a plural-arc container.
  */
template<typename ArcTraits>
struct SelectArcContainer<
  ArcTraits,
  typename std::enable_if<
    conjunction<
      typename ArcProperties<ArcTraits>::isExplicit,
      negation<typename ArcProperties<ArcTraits>::isOrdered>>::value,
    ArcTraits>::type> : public ExplicitArcs<ArcTraits>
{
  static_assert(ArcProperties<ArcTraits>::isExplicit::value, R"(
  Cannot use explicit arc storage for arcs that do not satisfy the explicit property.)");
  static_assert(
    !ArcProperties<ArcTraits>::isOrdered::value,
    "Ordered explicit arcs aren't yet supported.");
  using type = ExplicitArcs<ArcTraits>;

  SelectArcContainer() = default;
  SelectArcContainer(const ArcTraits& traits)
    : ExplicitArcs<ArcTraits>(traits)
  {
  }
};

/**\brief Store arcs explicitly with a random-access ordering.
  *
  * If an arc's traits object provides accessors/manipulators
  * required by its properties (i.e., directed/undirected,
  * forward/bidirectionally-indexed, singular/plural, etc.),
  * then use the storage as provided by the \a ArcTraits.
  */
template<typename ArcTraits>
struct SelectArcContainer<
  ArcTraits,
  typename std::enable_if<
    conjunction<
      typename ArcProperties<ArcTraits>::isExplicit,
      typename ArcProperties<ArcTraits>::isOrdered>::value,
    ArcTraits>::type> : public ExplicitArcs<ArcTraits> // ExplicitOrderedArcs
{
  static_assert(ArcProperties<ArcTraits>::isExplicit::value, R"(
  Cannot use explicit arc storage for arcs that do not satisfy the explicit property.)");
  static_assert(
    ArcProperties<ArcTraits>::isOrdered::value,
    "Cannot use ordered storage for unordered arcs.");
  using type = ExplicitArcs<ArcTraits>; // TODO: should be ExplicitOrderedArcs<ArcTraits>;

  SelectArcContainer() = default;
  SelectArcContainer(const ArcTraits& traits)
    : ExplicitArcs<ArcTraits>(traits)
  {
  }
};

/**\brief Store arcs implicitly.
  *
  * If an arc's traits object provides accessors/manipulators
  * required by its properties (i.e., directed/undirected,
  * forward/bidirectionally-indexed, singular/plural, etc.),
  * then use the storage as provided by the \a ArcTraits.
  */
template<typename ArcTraits>
struct SelectArcContainer<
  ArcTraits,
  typename std::enable_if<ArcProperties<ArcTraits>::isImplicit::value, ArcTraits>::type>
  : public ArcTraits
{
  static_assert(
    ArcProperties<ArcTraits>::isImplicit::value,
    "Cannot use implicit storage for this arc type.");
  using type = ArcTraits;

  SelectArcContainer() = default;
  SelectArcContainer(const ArcTraits& traits)
    : ArcTraits(traits)
  {
  }
};

} // namespace detail

namespace detail
{

template<typename Iterable>
class is_iterable
{
  template<typename X>
  static std::true_type testIterable(
    decltype(std::distance(std::declval<X>(), std::declval<X>()))*);
  template<typename X>
  static std::false_type testIterable(...);

public:
  using type = decltype(testIterable<Iterable>(nullptr));
  static constexpr bool value = type::value;
};

template<typename Container>
class is_container
{
  template<typename X>
  static std::true_type testContainer(
    decltype(std::distance(std::declval<X>().begin(), std::declval<X>().end()))*);
  template<typename X>
  static std::false_type testContainer(...);

public:
  using type = decltype(testContainer<Container>(nullptr));
  static constexpr bool value = type::value;
};

template<typename T, typename = void>
struct is_forward_iterable : public std::false_type
{
};

template<typename T>
struct is_forward_iterable<
  T,
  typename std::enable_if<std::is_base_of<
    std::forward_iterator_tag,
    typename std::iterator_traits<T>::iterator_category>::value>::type> : public std::true_type
{
};

template<typename T, typename = void>
struct is_iterable_container : public std::false_type
{
};

template<typename T>
struct is_iterable_container<
  T,
  typename std::enable_if<
    is_forward_iterable<decltype(std::declval<T>().begin())>::value &&
    is_forward_iterable<decltype(std::declval<T>().end())>::value>::type> : public std::true_type
{
};

template<typename API, typename Functor, typename Input>
class has_custom_visit
{
  template<typename X>
  static std::true_type testVisitable(
    decltype(std::declval<X>().visit(std::declval<Input>(), std::declval<const Functor&>()))*);
  template<typename X>
  static std::false_type testVisitable(...);

public:
  using type = decltype(testVisitable<API>(nullptr));
  static constexpr bool value = type::value;
};

template<typename Functor, typename Input>
class accepts
{
  template<typename X>
  static std::true_type testAccepts(decltype(std::declval<X>()(std::declval<Input>()))*);
  template<typename X>
  static std::false_type testAccepts(...);

public:
  using type = decltype(testAccepts<Functor>(nullptr));
  static constexpr bool value = type::value;
};

/// Provide default node storage as a NodeSet.
template<typename Traits, typename = void>
struct SelectNodeContainer
{
  using type = NodeSet;
};

/// If users provide a NodeContainer type-alias in the resource's type-traits, use it as storage.
template<typename Traits>
struct SelectNodeContainer<Traits, smtk::common::void_t<typename Traits::NodeContainer>>
{
  using type = typename Traits::NodeContainer;
};

template<typename Traits>
struct GraphTraits
{
  using NodeTypes = typename Traits::NodeTypes;
  using ArcTypes = typename Traits::ArcTypes;
  using NodeContainer = typename detail::SelectNodeContainer<Traits>::type;
};

} // namespace detail
} // namespace graph
} // namespace smtk

#endif
