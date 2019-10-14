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

#include <array>
#include <map>
#include <set>
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
namespace detail
{
static constexpr const char* const char_name = "char";
static constexpr const char* const unsigned_char_name = "unsigned char";
static constexpr const char* const signed_char_name = "signed char";
static constexpr const char* const int_name = "int";
static constexpr const char* const unsigned_int_name = "unsigned int";
static constexpr const char* const signed_int_name = "signed int";
static constexpr const char* const short_int_name = "short int";
static constexpr const char* const unsigned_short_int_name = "unsigned short int";
static constexpr const char* const signed_short_int_name = "signed short int";
static constexpr const char* const long_int_name = "long int";
static constexpr const char* const signed_long_int_name = "signed long int";
static constexpr const char* const unsigned_long_int_name = "unsigned long int";
static constexpr const char* const float_name = "float";
static constexpr const char* const double_name = "double";
static constexpr const char* const long_double_name = "long double";
static constexpr const char* const wchar_t_name = "wchar_t";
static constexpr const char* const std_string_name = "string";
}

/// For serialization across platforms, we define platform-neutral type names
/// for plain-old-datatypes (PODs) and for std::string.
typedef std::tuple<char, unsigned char, signed char, int, unsigned int, signed int, short int,
  unsigned short int, signed short int, long int, signed long int, unsigned long int, float, double,
  long double, wchar_t, std::string>
  PODs;

typedef std::array<const char* const, std::tuple_size<PODs>::value> PODNames_t;
static constexpr PODNames_t PODNames = { { detail::char_name, detail::unsigned_char_name,
  detail::signed_char_name, detail::int_name, detail::unsigned_int_name, detail::signed_int_name,
  detail::short_int_name, detail::unsigned_short_int_name, detail::signed_short_int_name,
  detail::long_int_name, detail::signed_long_int_name, detail::unsigned_long_int_name,
  detail::float_name, detail::double_name, detail::long_double_name, detail::wchar_t_name,
  detail::std_string_name } };

/// Resources and operations have a virtual method typeName(),  but to access
/// this value we must create an instance of the class. Alternatively, these
/// classes can (and should) declare a constant expression field "type_name"
/// that can be queried  without instantiating this class. The macro
/// "smtkTypeMacro" is defined in SharedFromThis.h to declare this field in
/// tandem with typeName(). To relax the requirements of a) a macro definition
/// in a class header, or b) a mysterious constexpr in a class declaration,
/// the free function name() will traverse one of two code paths to determine a
/// type name for a user-defined type.

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

// A compile-time test to check whether or not a type is in our list of PODs.
template <typename T>
struct is_pod
{
  using type = typename smtk::tuple_has<T, PODs>::type;
};

// A compile-time test to check whether or not a class has a create() method.
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

// The signature for our name-finding struct has four template parameters.
template <typename Type, typename is_named, typename is_pod, typename is_constructible>
struct name;

// This partial template specialization deals with the case where
// <Type> has a type_name.
template <typename Type, typename is_pod, typename is_constructible>
struct name<Type, std::true_type, is_pod, is_constructible>
{
  static std::string value() { return Type::type_name; }
};

// This partial template specialization deals with the case where
// <Type> is a POD.
template <typename Type, typename is_constructible>
struct name<Type, std::false_type, std::true_type, is_constructible>
{
  static std::string value() { return std::string(PODNames.at(tuple_index<Type, PODs>::value)); }
};

// This partial template specialization deals with the case where
// <Type> does not have a type_name and is not a POD, but can be created.
template <typename Type>
struct name<Type, std::false_type, std::false_type, std::true_type>
{
  static std::string value() { return Type::create()->typeName(); }
};

// As a last resort, we return the machine-dependent name for the type.
template <typename Type>
struct name<Type, std::false_type, std::false_type, std::false_type>
{
  static std::string value() { return typeid(Type).name(); }
};

// This partial template specialization provides support for vectors of named
// types.
template <typename Type>
struct name<std::vector<Type>, std::false_type, std::false_type, std::false_type>
{
  static std::string value()
  {
    std::string subtype = name<Type, typename detail::is_named<Type>::type,
      typename detail::is_pod<Type>::type, typename detail::is_constructible<Type>::type>::value();
    return std::string("vector<" + subtype + ">");
  }
};

// This partial template specialization provides support for sets of named
// types.
template <typename Type>
struct name<std::set<Type>, std::false_type, std::false_type, std::false_type>
{
  static std::string value()
  {
    std::string subtype = name<Type, typename detail::is_named<Type>::type,
      typename detail::is_pod<Type>::type, typename detail::is_constructible<Type>::type>::value();
    return std::string("set<" + subtype + ">");
  }
};

// This partial template specialization provides support for unordered sets of
// named types.
template <typename Type>
struct name<std::unordered_set<Type>, std::false_type, std::false_type, std::false_type>
{
  static std::string value()
  {
    std::string subtype = name<Type, typename detail::is_named<Type>::type,
      typename detail::is_pod<Type>::type, typename detail::is_constructible<Type>::type>::value();
    return std::string("unordered set<" + subtype + ">");
  }
};

// This partial template specialization provides support for maps of named
// types.
template <typename KeyType, typename ValueType>
struct name<std::map<KeyType, ValueType>, std::false_type, std::false_type, std::false_type>
{
  static std::string value()
  {
    std::string keytype = name<KeyType, typename detail::is_named<KeyType>::type,
      typename detail::is_pod<KeyType>::type,
      typename detail::is_constructible<KeyType>::type>::value();
    std::string valuetype = name<ValueType, typename detail::is_named<ValueType>::type,
      typename detail::is_pod<ValueType>::type,
      typename detail::is_constructible<ValueType>::type>::value();
    return std::string("map<" + keytype + ", " + valuetype + ">");
  }
};

// This partial template specialization provides support for unordered maps of
// named types.
template <typename KeyType, typename ValueType>
struct name<std::unordered_map<KeyType, ValueType>, std::false_type, std::false_type,
  std::false_type>
{
  static std::string value()
  {
    std::string keytype = name<KeyType, typename detail::is_named<KeyType>::type,
      typename detail::is_pod<KeyType>::type,
      typename detail::is_constructible<KeyType>::type>::value();
    std::string valuetype = name<ValueType, typename detail::is_named<ValueType>::type,
      typename detail::is_pod<ValueType>::type,
      typename detail::is_constructible<ValueType>::type>::value();
    return std::string("unordered map<" + keytype + ", " + valuetype + ">");
  }
};
}

/// Return the name of a class.
template <typename Type>
std::string typeName()
{
  return detail::name<Type, typename detail::is_named<Type>::type,
    typename detail::is_pod<Type>::type, typename detail::is_constructible<Type>::type>::value();
}
}
}

#endif // smtk_common_TypeName_h
