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

#include <string>
#include <typeinfo>

namespace smtk
{
namespace common
{

/// Resources and operations have a virtual method typeName(),  but to access
/// this value we must create an instance of the class. Alternatively, these
/// classes can (and should) declare a constant expression field "type_name"
/// that can be queried  without instantiating this class. The macro
/// "smtkTypeMacro" is defined in SharedFromThis.h to declare this field in
/// tandem with typeName(). To relax the requirements of a) a macro definition
/// in a class header, or b) a mysterious constexpr in a class declaration,
/// the free function name() will traverse one of two code paths to determine a
/// type name.

namespace detail
{
// A compile-time test to check whether or not a class has a type_name defined.
template <typename T>
class is_named
{
  template <typename X>
  static std::true_type testNamed(decltype(X::type_name)*);
  template <typename X>
  static std::false_type testNamed(...);

public:
  using type = decltype(testNamed<T>(nullptr));
};

template <typename T>
class is_constructible
{
  template <typename X>
  static std::true_type testConstructible(decltype(X::create())*);
  template <typename X>
  static std::false_type testConstructible(...);

public:
  using type = decltype(testConstructible<T>(nullptr));
};

// The signature for our name-finding struct has two template parameters.
template <typename Type, typename is_named>
struct name;

// This partial template specialization deals with the case where
// <Type> does not have a type_name. In this case, we create a temporary
// object and ask for its typeName.
template <typename Type>
struct name<Type, std::false_type>
{

  static typename std::enable_if<is_constructible<Type>::type, std::string>::type value()
  {
    return Type::create()->typeName();
  }

  // As a last resort, we return the machine-dependent name for tye type.
  static typename std::enable_if<!is_constructible<Type>::type, std::string>::type value(int i = 0)
  {
    (void)i;
    return typeid(Type).name();
  }
};

// This partial template specialization deals with the case where
// <Type> has a type_name. In this case, we can return the class type
// name without instantiating the class.
template <typename Type>
struct name<Type, std::true_type>
{
  static std::string value() { return Type::type_name; }
};
}

/// Return the name of a class.
template <typename Type>
std::string typeName()
{
  return detail::name<Type, typename detail::is_named<Type>::type>::value();
}
}
}

#endif // smtk_common_TypeName_h
