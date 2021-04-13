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

/// @file TupleTraits.h Tuple convenience functions
namespace smtk
{
namespace detail
{
/// If T is not in Tuple, return std::tuple<T>. Otherwise, return std::tuple<>.
///
/// Examples:
/// ```
///   tuple_if_unique<int, std::tuple<float, double>::type == std::tuple<int>
///   tuple_if_unique<int, std::tuple<int, double>::type == std::tuple<>
/// ```
template<typename T, typename Tuple>
struct tuple_if_unique;

/// @see tuple_if_unique
template<typename T>
struct tuple_if_unique<T, std::tuple<>>
{
  using type = std::tuple<T>;
};

/// @see tuple_if_unique
template<typename T, typename U, typename... Ts>
struct tuple_if_unique<T, std::tuple<U, Ts...>>
{
  using type = typename tuple_if_unique<T, std::tuple<Ts...>>::type;
};

/// @see tuple_if_unique
template<typename T, typename... Ts>
struct tuple_if_unique<T, std::tuple<T, Ts...>>
{
  using type = std::tuple<>;
};

/// If T is not a base of any types in Tuple, return std::tuple<T>. Otherwise,
/// return std::tuple<>.
template<typename T, typename Tuple>
struct tuple_if_not_base;

/// @see tuple_if_not_base
template<typename T>
struct tuple_if_not_base<T, std::tuple<>>
{
  using type = std::tuple<T>;
};

/// @see tuple_if_not_base
template<typename T, typename U, typename... Ts>
struct tuple_if_not_base<T, std::tuple<U, Ts...>>
{
  using type = typename std::conditional<
    std::is_base_of<T, U>::value,
    std::tuple<>,
    typename tuple_if_not_base<T, std::tuple<Ts...>>::type>::type;
};

/// Given a tuple, starting with the first parameter and moving to the last,
/// remove the parameter from the tuple if it is a base of any of the parameters
/// to the right.
template<typename T, typename... Args>
struct remove_bases_from_tuple_lr;

template<>
struct remove_bases_from_tuple_lr<std::tuple<>>
{
  using type = std::tuple<>;
};

template<typename T>
struct remove_bases_from_tuple_lr<std::tuple<T>>
{
  using type = std::tuple<T>;
};

template<typename T, typename... Args>
struct remove_bases_from_tuple_lr<std::tuple<T, Args...>>
{
  using type = decltype(std::tuple_cat(
    std::declval<typename detail::tuple_if_not_base<T, std::tuple<Args...>>::type>(),
    std::declval<typename remove_bases_from_tuple_lr<std::tuple<Args...>>::type>()));
};

/// If T is a tuple containing a single type, return the type it contians.
/// Otherwise, return T.
///
/// Examples:
/// ```
///   untuple<int>::type == int
///   untuple<std::tuple<int>::type == int
/// ```
template<typename T>
struct untuple
{
  using type = T;
};

/// @see untuple
template<typename T>
struct untuple<std::tuple<T>>
{
  using type = typename untuple<T>::type;
};
} // namespace detail

/// Takes a tuple of tuples and types and returns a tuple of the tuple contents
/// and the plain types.
///
/// Examples:
/// ```
///   flatten_tuple<std::tuple<int, float>>::type == std::tuple<int, float>
///   flatten_tuple<std::tuple<int, std::tuple<float, float>>>::type ==
///     std::tuple<int, float, float>
/// ```
template<typename T>
struct flatten_tuple;

/// @see flatten_tuple
template<>
struct flatten_tuple<std::tuple<>>
{
  using type = std::tuple<>;
};

/// @see flatten_tuple
template<typename T, typename... Args>
struct flatten_tuple<std::tuple<T, Args...>>
{
  using type = decltype(std::tuple_cat(
    std::declval<std::tuple<typename detail::untuple<T>::type>>(),
    std::declval<typename flatten_tuple<std::tuple<Args...>>::type>()));
};

/// @see my_flatten_tuple
template<typename... TupleArgs, typename... Args>
struct flatten_tuple<std::tuple<std::tuple<TupleArgs...>, Args...>>
{
  using type = decltype(std::tuple_cat(
    std::declval<std::tuple<TupleArgs...>>(),
    std::declval<typename flatten_tuple<std::tuple<Args...>>::type>()));
};

/// Takes a tuple of potentially duplicate types and returns a tuple with the
/// duplicate types removed.
///
/// Examples:
/// ```
///   unique_tuple<std::tuple<int, float>>::type == std::tuple<int, float>
///   unique_tuple<std::tuple<int, int, float>>::type == std::tuple<int, float>
/// ```
template<typename T, typename... Args>
struct unique_tuple;

/// @see unique_tuple
template<>
struct unique_tuple<std::tuple<>>
{
  using type = std::tuple<>;
};

/// @see unique_tuple
template<typename T>
struct unique_tuple<std::tuple<T>>
{
  using type = std::tuple<T>;
};

/// @see unique_tuple
template<typename T, typename... Args>
struct unique_tuple<std::tuple<T, Args...>>
{
  using type = decltype(std::tuple_cat(
    std::declval<typename detail::tuple_if_unique<T, std::tuple<Args...>>::type>(),
    std::declval<typename unique_tuple<std::tuple<Args...>>::type>()));
};

/// Takes a tuple and reverses its arguments.
///
/// Examples:
/// ```
///   reverse_tuple<std::tuple<bool, int, float>>::type == std::tuple<float, int, bool>
/// ```
template<typename T>
struct reverse_tuple;

template<typename T>
struct reverse_tuple<std::tuple<T>>
{
  using type = std::tuple<T>;
};

template<typename T, typename... Args>
struct reverse_tuple<std::tuple<T, Args...>>
{
  using type = decltype(std::tuple_cat(
    std::declval<typename reverse_tuple<std::tuple<Args...>>::type>(),
    std::declval<std::tuple<T>>()));
};

/// Takes a template class and a tuple and returns a tuple of elements
/// transformed by the template class, recursing through nested tuples if
/// necessary.
///
/// Examples:
/// ```
///   recursive<std::remove_reference, std::tuple<bool&, std::tuple<int&, float&> >::type ==
///    std::tuple<bool, std::tuple<int, float> >
/// ```
template<template<typename> class X, typename T>
struct recursive
{
  using type = typename X<T>::type;
};

template<template<typename> class X, typename... Args>
struct recursive<X, std::tuple<Args...>>
{
  using type = typename X<std::tuple<typename recursive<X, Args>::type...>>::type;
};

/// Takes a tuple and removes any types that are bases of any other types in the
/// tuple.
template<typename T>
struct remove_bases_from_tuple
{
  using type = typename detail::remove_bases_from_tuple_lr<typename reverse_tuple<
    typename detail::remove_bases_from_tuple_lr<typename reverse_tuple<T>::type>::type>::type>::
    type;
};

/// Takes a type and a tuple of types and returns a tuple with all of the
/// original types sans the input type.
///
/// Examples:
/// ```
///   remove_from_tuple<bool, std::tuple<int, float>>::type ==
///      std::tuple<int, float>
///   remove_from_tuple<int, std::tuple<int, float>>::type == std::tuple<float>
/// ```
template<typename T, typename Tuple>
struct remove_from_tuple;

/// @see remove_from_tuple
template<typename T, typename... Args>
struct remove_from_tuple<T, std::tuple<T, Args...>>
{
  using type = decltype(std::declval<typename remove_from_tuple<T, std::tuple<Args...>>::type>());
};

/// @see remove_from_tuple
template<typename T, typename X, typename... Args>
struct remove_from_tuple<T, std::tuple<X, Args...>>
{
  using type = decltype(std::tuple_cat(
    std::declval<std::tuple<X>>(),
    std::declval<typename remove_from_tuple<T, std::tuple<Args...>>::type>()));
};

/// @see remove_from_tuple
template<typename T>
struct remove_from_tuple<T, std::tuple<>>
{
  using type = std::tuple<>;
};

/// Takes a type and a tuple of types and returns the index of the first
/// instance of that type in the tuple.
///
/// Examples:
/// ```
///   tuple_index<bool, std::tuple<int, bool, float>>::value == 1
/// ```
template<class T, class Tuple>
struct tuple_index;

/// @see tuple_index
template<class T, class... Types>
struct tuple_index<T, std::tuple<T, Types...>>
{
  constexpr static const std::size_t value = 0;
};

/// @see tuple_index
template<class T, class U, class... Types>
struct tuple_index<T, std::tuple<U, Types...>>
{
  constexpr static const std::size_t value = 1 + tuple_index<T, std::tuple<Types...>>::value;
};

/// Takes a type and a tuple of types and returns a bool indicating whether or
/// not the type is in the tuple.
///
/// Examples:
/// ```
///   tuple_contains<bool, std::tuple<int, bool, float>>() == true
///   tuple_contains<bool, std::tuple<int, float>>() == false
/// ```
template<typename T, typename Tuple>
struct tuple_contains;

/// @see tuple_contains
template<typename T>
struct tuple_contains<T, std::tuple<>> : std::false_type
{
};

/// @see tuple_contains
template<typename T, typename U, typename... Ts>
struct tuple_contains<T, std::tuple<U, Ts...>> : tuple_contains<T, std::tuple<Ts...>>
{
};

/// @see tuple_contains
template<typename T, typename... Ts>
struct tuple_contains<T, std::tuple<T, Ts...>> : std::true_type
{
};

/// Embeds a type in another class so its type information can be passed as a
/// parameter.
template<typename T>
struct identity
{
  typedef T type;
};

template<std::size_t...>
struct sequence
{
};

template<std::size_t N, std::size_t... S>
struct index_sequence : index_sequence<N - 1, N - 1, S...>
{
};

template<std::size_t... S>
struct index_sequence<0, S...>
{
  typedef sequence<S...> type;
};
} // namespace smtk

#endif
