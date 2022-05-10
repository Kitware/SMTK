//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_Metaprogramming_h
#define smtk_Metaprogramming_h

#include <functional>
#include <iterator>
#include <set>
#include <type_traits>

namespace smtk
{

/**\brief True when all template parameters are "truthy".
  *
  * This is a polyfill for std::conjunction, introduced in c++17.
  */
//@{
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
//@}

/**\brief True when any template parameter is "truthy".
  *
  * This is a polyfill for std::disjunction, introduced in c++17.
  */
//@{
template<class...>
struct disjunction : std::false_type
{
};
template<class B1>
struct disjunction<B1> : B1
{
};
template<class B1, class... Bn>
struct disjunction<B1, Bn...> : std::conditional<bool(B1::value), B1, disjunction<Bn...>>::type
{
};
//@}

/// A "polyfill" for std::negation, introduced in c++17.
template<class B>
struct negation : std::integral_constant<bool, !bool(B::value)>
{
};

/// Select between two types with a boolean template parameter.
//@{
template<bool selector, typename type_when_selector_true, typename type_when_selector_false>
struct type_switch
{
  using type = type_when_selector_true;
};

template<typename type_when_selector_true, typename type_when_selector_false>
struct type_switch<false, type_when_selector_true, type_when_selector_false>
{
  using type = type_when_selector_false;
};
//@}

} // namespace smtk

#endif
