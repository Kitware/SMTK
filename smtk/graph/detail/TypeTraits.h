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

#include <functional>
#include <iterator>
#include <set>
#include <type_traits>

namespace smtk
{
namespace graph
{

class NodeSet;

template<typename FromType, typename ToType, typename InverseArc>
class OrderedArcs;

namespace detail
{
template<typename ArcType, typename = void>
struct ArcInverse
{
  using type = void;
};

template<typename ArcType>
struct ArcInverse<ArcType, smtk::common::void_t<typename ArcType::InverseArcType>>
{
  using type = typename ArcType::InverseArcType;
};
} // namespace detail

// Public traits
template<typename ArcType>
struct ArcTraits
{
  using InverseArcType = typename detail::ArcInverse<ArcType>::type;
  using FromType = typename smtk::common::remove_cvref<typename ArcType::FromType>::type;
  using ToType = typename smtk::common::remove_cvref<typename ArcType::ToType>::type;
};

namespace detail
{

template<class...>
struct conjunction : std::true_type
{
};
template<class B1>
struct conjunction<B1> : B1
{
};
template<class B1, class... Bn>
struct conjunction<B1, Bn...> : std::conditional<bool(B1::value), conjunction<Bn...>, B1>::type
{
};

template<typename T, typename... Ts>
using CompatibleTypes =
  typename std::enable_if<conjunction<std::is_convertible<Ts, T>...>::value>::type;

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

template<typename Traits, typename = void>
struct SelectNodeContainer
{
  using type = NodeSet;
};

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

template<typename ArcType, typename = void>
struct is_ordered_arcs : std::false_type
{
};

template<typename ArcType>
struct is_ordered_arcs<
  ArcType,
  typename std::enable_if<std::is_base_of<
    OrderedArcs<typename ArcType::FromType, typename ArcType::ToType, ArcType>,
    ArcType>::value>::type> : std::true_type
{
};

template<typename ArcType, typename = void>
struct has_inverse : std::false_type
{
};

template<typename ArcType>
struct has_inverse<
  ArcType,
  typename std::enable_if<!std::is_same<typename ArcInverse<ArcType>::type, void>::value>::type>
  : std::true_type
{
};

template<typename ArcType, typename OtherArcType, typename = void>
struct is_inverse_pair : std::false_type
{
};

template<typename ArcType, typename OtherArcType>
struct is_inverse_pair<
  ArcType,
  OtherArcType,
  typename std::enable_if<
    // Inverse Arc Type should not be void
    has_inverse<ArcType>::value &&      //
    has_inverse<OtherArcType>::value && //
    // To/From Types should match
    std::is_same<typename ArcType::FromType, typename OtherArcType::ToType>::value && //
    std::is_same<typename ArcType::ToType, typename OtherArcType::FromType>::value && //
    // Inverse Arc Types should match (but apparently this is information that is not yet available
    std::is_same<typename ArcInverse<ArcType>::type, OtherArcType>::value && //
    std::is_same<ArcType, typename ArcInverse<OtherArcType>::type>::value    //
    >::type> : std::true_type
{
};

} // namespace detail
} // namespace graph
} // namespace smtk

#endif
