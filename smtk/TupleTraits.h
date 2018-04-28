//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_TupleTraits_h
#define __smtk_TupleTraits_h

#include <tuple>

namespace smtk
{
namespace detail
{
/// If T is in Tuple, return std::tuple<T>. Otherwise, return std::tuple<>.
///
/// Examples:
///   tuple_if_unique<int, std::tuple<float, double>::type == std::tuple<int>
///   tuple_if_unique<int, std::tuple<int, double>::type == std::tuple<>
template <typename T, typename Tuple>
struct tuple_if_unique;

template <typename T>
struct tuple_if_unique<T, std::tuple<> >
{
  using type = std::tuple<T>;
};

template <typename T, typename U, typename... Ts>
struct tuple_if_unique<T, std::tuple<U, Ts...> >
{
  using type = typename tuple_if_unique<T, std::tuple<Ts...> >::type;
};

template <typename T, typename... Ts>
struct tuple_if_unique<T, std::tuple<T, Ts...> >
{
  using type = std::tuple<>;
};

/// If T is a tuple containing a single type, return the type it contians.
/// Otherwise, return T.
///
/// Examples:
///   untuple<int>::type == int
///   untuple<std::tuple<int>::type == int
template <typename T>
struct untuple
{
  using type = T;
};

template <typename T>
struct untuple<std::tuple<T> >
{
  using type = typename untuple<T>::type;
};
}

/// Takes a tuple of tuples and types and returns a tuple of the tuple contents
/// and the plain types.
///
/// Examples:
///   flatten_tuple<std::tuple<int, float>>::type == std::tuple<int, float>
///   flatten_tuple<std::tuple<int, std::tuple<float, float>>>::type ==
///     std::tuple<int, float, float>
template <typename T>
struct flatten_tuple;

template <>
struct flatten_tuple<std::tuple<> >
{
  using type = std::tuple<>;
};

template <typename T, typename... Args>
struct flatten_tuple<std::tuple<T, Args...> >
{
  using type = decltype(
    std::tuple_cat(std::declval<std::tuple<typename detail::untuple<T>::type> >(),
      std::declval<typename flatten_tuple<std::tuple<Args...> >::type>()));
};

/// Takes a tuple of potentially duplicate types and returns a tuple with the
/// duplicate types removed.
///
/// Examples:
///   unique_tuple<std::tuple<int, float>>::type == std::tuple<int, float>
///   unique_tuple<std::tuple<int, int, float>>::type == std::tuple<int, float>
template <typename T, typename... Args>
struct unique_tuple;

template <>
struct unique_tuple<std::tuple<> >
{
  using type = std::tuple<>;
};

template <typename T>
struct unique_tuple<std::tuple<T> >
{
  using type = std::tuple<T>;
};

template <typename T, typename... Args>
struct unique_tuple<std::tuple<T, Args...> >
{
  using type = decltype(
    std::tuple_cat(std::declval<typename detail::tuple_if_unique<T, std::tuple<Args...> >::type>(),
      std::declval<typename unique_tuple<std::tuple<Args...> >::type>()));
};

/// Takes a type and a tuple of types and returns a tuple with all of the
/// original types sans the input type.
///
/// Examples:
///   remove_from_tuple<bool, std::tuple<int, float>>::type ==
///     std::tuple<int, float>
///   remove_from_tuple<int, std::tuple<int, float>>::type == std::tuple<float>
template <typename T, typename Tuple>
struct remove_from_tuple;

template <typename T, typename... Args>
struct remove_from_tuple<T, std::tuple<T, Args...> >
{
  using type = decltype(std::declval<typename remove_from_tuple<T, std::tuple<Args...> >::type>());
};

template <typename T, typename X, typename... Args>
struct remove_from_tuple<T, std::tuple<X, Args...> >
{
  using type = decltype(std::tuple_cat(std::declval<std::tuple<X> >(),
    std::declval<typename remove_from_tuple<T, std::tuple<Args...> >::type>()));
};

template <typename T>
struct remove_from_tuple<T, std::tuple<> >
{
  using type = std::tuple<>;
};
}

#endif
