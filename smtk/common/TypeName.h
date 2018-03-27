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

namespace smtk
{
namespace common
{

/// Resources and operations have a virtual method typeName(),  but to access
/// this value we  must create an instance of the class. Alternatively, these
/// classes can (and should) declare a constant expression field "type_name"
/// that can be queried  without instantiating this class. The macro
/// "smtkTypeMacro" defined in SharedFromThis.h to declare this field in
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
  class No
  {
  };
  class Yes
  {
    No no[2];
  };

  template <typename C>
  static Yes Test(typename C::type_name*);
  template <typename C>
  static No Test(...);

public:
  enum
  {
    value = sizeof(Test<T>(0)) == sizeof(Yes)
  };
};

// The signature for our name-finding struct has two template parameters.
template <typename Type, bool is_named>
struct name;

// This partial template specialization deals with the case where
// <Type> does not have a type_name. In this case, we create a temporary
/// object and ask for its typeName.
template <typename Type>
struct name<Type, false>
{
  static std::string value() { return Type::create()->typeName(); }
};

// This partial template specialization deals with the case where
// <Type> has a type_name. In this case, we can return the class type
// name without instantiating the class.
template <typename Type>
struct name<Type, true>
{
  static std::string value() { return Type::type_name; }
};
}

/// Return the name of a class.
template <typename Type>
std::string typeName()
{
  return detail::name<Type, detail::is_named<Type>::value>::value();
}
}
}

#endif // smtk_common_TypeName_h
