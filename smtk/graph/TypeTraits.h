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

#include "smtk/graph/NodeSet.h"

#include "smtk/common/TypeTraits.h"

#include <functional>
#include <set>
#include <type_traits>

namespace smtk
{
namespace graph
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

namespace detail
{
template<typename Traits, typename = void>
struct SelectNodesStorage
{
  using type = NodeSet;
};

template<typename Traits>
struct SelectNodesStorage<Traits, smtk::common::void_t<typename Traits::NodeStorage>>
{
  using type = typename Traits::NodeStorage;
};

} // namespace detail

template<typename Traits>
struct GraphTraits
{
  using NodeTypes = typename Traits::NodeTypes;
  using ArcTypes = typename Traits::ArcTypes;
  using NodeStorage = typename detail::SelectNodesStorage<Traits>::type;
};

} // namespace graph
} // namespace smtk

#endif
