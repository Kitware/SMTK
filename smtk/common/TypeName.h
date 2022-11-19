//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_common_TypeName_h
#define smtk_common_TypeName_h

#include "smtk/TupleTraits.h"

#include "smtk/common/CompilerInformation.h"

#include <boost/type_index.hpp>

#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <tuple>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace smtk
{
namespace common
{
/// @file TypeName.h \brief Named type functions.
///
/// Resources and operations have a virtual method typeName(), but to access
/// this value we must create an instance of the class. Alternatively, these
/// classes can (and should) declare a constant expression field "type_name"
/// that can be queried without instantiating this class. The macro
/// "smtkTypeMacro" is defined in SharedFromThis.h to declare this field in
/// tandem with typeName(). To relax the requirements of a) a macro definition
/// in a class header, or b) a mysterious constexpr in a class declaration,
/// the free function name() will traverse one of two code paths to determine a
/// type name for a user-defined type.

namespace detail
{
/// A compile-time test to check whether or not a class has a type_name defined.
template<typename T>
class is_named
{
  template<typename X>
  static std::true_type testNamed(decltype(X::type_name)*);
  template<typename X>
  static std::false_type testNamed(...);

public:
  using type = decltype(testNamed<T>(nullptr));
};

template<typename Type>
struct name
{
  // If there's a user-defined type_name field, use it.
  template<typename T>
  static typename std::enable_if<is_named<T>::type::value, std::string>::type value_()
  {
    return T::type_name;
  }

  /// By default, we return the prettified type name for the type.
  template<typename T>
  static typename std::enable_if<!is_named<T>::type::value, std::string>::type value_()
  {
#ifdef SMTK_MSVC
    // MSVC's implementation of type_name refers to classes as "class foo". To
    // maintain parity with other compilers, we strip the preceding "class "
    // away. We do the same for "struct ".
    std::string pretty_name = boost::typeindex::type_id<Type>().pretty_name();
    if (pretty_name.substr(0, 6) == "class ")
    {
      pretty_name = pretty_name.substr(6);
    }
    else if (pretty_name.substr(0, 7) == "struct ")
    {
      pretty_name = pretty_name.substr(7);
    }
    // MSVC also uses "`anonymous namespace'" instead of "(anonymous namespace)".
    // Make that match as well.
    if (pretty_name.substr(0, 21) == "`anonymous namespace'")
    {
      pretty_name = "(anonymous namespace)" + pretty_name.substr(21);
    }
    return pretty_name;
#else
    return boost::typeindex::type_id<Type>().pretty_name();
#endif
  }

  static std::string value() { return value_<Type>(); }
};

// Specialization for std::string.
template<>
struct name<std::string>
{
  static std::string value() { return "string"; }
};

// Specialization for std::tuple.
template<typename... Types>
struct name<std::tuple<Types...>>
{
  static std::string value()
  {
    std::string subtypes = subType<0, std::tuple<Types...>>();
    return std::string("tuple<" + subtypes + ">");
  }

  template<std::size_t I, typename Tuple>
  inline static typename std::enable_if<I != std::tuple_size<Tuple>::value, std::string>::type
  subType()
  {
    typedef typename std::tuple_element<I, Tuple>::type Type;
    std::string subtype = name<Type>::value();

    return (I != 0 ? std::string(", ") : std::string()) + subtype + subType<I + 1, Tuple>();
  }

  template<std::size_t I, typename Tuple>
  inline static typename std::enable_if<I == std::tuple_size<Tuple>::value, std::string>::type
  subType()
  {
    return std::string();
  }
};

// Specialization for std::array.
template<typename Type, size_t N>
struct name<std::array<Type, N>>
{
  static std::string value()
  {
    std::string subtype = name<Type>::value();
    return std::string("array<" + subtype + "," + std::to_string(N) + ">");
  }
};

// Specialization for std::priority_queue.
template<typename Type>
struct name<std::priority_queue<Type, std::vector<Type>, std::less<Type>>>
{
  static std::string value()
  {
    std::string subtype = name<Type>::value();

    return std::string("priority_queue<" + subtype + ">");
  }
};

// Specialization for smart pointers.
#define name_single_argument_stl_pointer(POINTER)                                                  \
  template<typename Type>                                                                          \
  struct name<std::POINTER<Type>>                                                                  \
  {                                                                                                \
    static std::string value()                                                                     \
    {                                                                                              \
      std::string subtype = name<Type>::value();                                                   \
      return std::string(#POINTER) + "<" + subtype + ">";                                          \
    }                                                                                              \
  }

name_single_argument_stl_pointer(shared_ptr);
name_single_argument_stl_pointer(weak_ptr);
name_single_argument_stl_pointer(unique_ptr);

// Specialization for containers with allocators.
#undef name_single_argument_stl_container

#define name_single_argument_stl_container(CONTAINER)                                              \
  template<typename Type>                                                                          \
  struct name<std::CONTAINER<Type, std::allocator<Type>>>                                          \
  {                                                                                                \
    static std::string value()                                                                     \
    {                                                                                              \
      std::string subtype = name<Type>::value();                                                   \
      return std::string(#CONTAINER) + "<" + subtype + ">";                                        \
    }                                                                                              \
  }

name_single_argument_stl_container(vector);
name_single_argument_stl_container(deque);
name_single_argument_stl_container(forward_list);
name_single_argument_stl_container(list);

#undef name_single_argument_stl_container

// Specialization for containers with allocators and comparators.
#define name_single_argument_sorted_stl_container(CONTAINER)                                       \
  template<typename Type>                                                                          \
  struct name<std::CONTAINER<Type, std::less<Type>, std::allocator<Type>>>                         \
  {                                                                                                \
    static std::string value()                                                                     \
    {                                                                                              \
      std::string subtype = name<Type>::value();                                                   \
      return std::string(#CONTAINER) + "<" + subtype + ">";                                        \
    }                                                                                              \
  }

name_single_argument_sorted_stl_container(set);
name_single_argument_sorted_stl_container(multiset);
name_single_argument_sorted_stl_container(unordered_set);
name_single_argument_sorted_stl_container(unordered_multiset);

#undef name_single_argument_sorted_stl_container

// Specialization for containers that accept a type and container type.
#define name_double_argument_stl_container(CONTAINER)                                              \
  template<typename Type>                                                                          \
  struct name<std::CONTAINER<Type, std::deque<Type>>>                                              \
  {                                                                                                \
    static std::string value()                                                                     \
    {                                                                                              \
      std::string type = name<Type>::value();                                                      \
      return std::string(#CONTAINER) + "<" + type + ">";                                           \
    }                                                                                              \
  }

name_double_argument_stl_container(queue);
name_double_argument_stl_container(stack);

#undef name_double_argument_stl_container

// Specialization for containers that accept a key, value, comparator and allocator.
#define name_double_argument_sorted_stl_container(CONTAINER)                                       \
  template<typename KeyType, typename ValueType>                                                   \
  struct name<std::CONTAINER<                                                                      \
    KeyType,                                                                                       \
    ValueType,                                                                                     \
    std::less<KeyType>,                                                                            \
    std::allocator<std::pair<const KeyType, ValueType>>>>                                          \
  {                                                                                                \
    static std::string value()                                                                     \
    {                                                                                              \
      std::string keytype = name<KeyType>::value();                                                \
      std::string valuetype = name<ValueType>::value();                                            \
      return std::string(#CONTAINER) + "<" + keytype + ", " + valuetype + ">";                     \
    }                                                                                              \
  }

name_double_argument_sorted_stl_container(map);
name_double_argument_sorted_stl_container(multimap);

// Specialization for containers that accept a key, value, comparator and allocator.
#define name_double_argument_hashed_stl_container(CONTAINER)                                       \
  template<typename KeyType, typename ValueType>                                                   \
  struct name<std::CONTAINER<                                                                      \
    KeyType,                                                                                       \
    ValueType,                                                                                     \
    std::hash<KeyType>,                                                                            \
    std::equal_to<KeyType>,                                                                        \
    std::allocator<std::pair<const KeyType, ValueType>>>>                                          \
  {                                                                                                \
    static std::string value()                                                                     \
    {                                                                                              \
      std::string keytype = name<KeyType>::value();                                                \
      std::string valuetype = name<ValueType>::value();                                            \
      return std::string(#CONTAINER) + "<" + keytype + ", " + valuetype + ">";                     \
    }                                                                                              \
  }

name_double_argument_hashed_stl_container(unordered_map);
name_double_argument_hashed_stl_container(unordered_multimap);

#undef name_double_argument_hashed_stl_container
} // namespace detail

/// Return the name of a class.
template<typename Type>
std::string typeName()
{
  return detail::name<Type>::value();
}
} // namespace common
} // namespace smtk

#endif // smtk_common_TypeName_h
