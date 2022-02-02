//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_common_TypeTraits_h
#define smtk_common_TypeTraits_h

#include <type_traits>

namespace smtk
{
namespace common
{

// Added in C++17
template<class...>
using void_t = void;

// Added in C++20
template<class T>
struct remove_cvref
{
  using type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
};

} // namespace common
} // namespace smtk

#endif // smtk_common_TypeTraits_h
