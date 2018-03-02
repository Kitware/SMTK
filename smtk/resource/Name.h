//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_resource_Name_h
#define smtk_resource_Name_h

#include <string>

namespace smtk
{
namespace resource
{

/// Resources have a virtual method uniqueName(),  but to access this value we
/// must create an instance of the resource. Alternatively, resources can (and
/// should) declare a constant expression field "type_name" that can be queried
/// without instantiating this resource. There is a macro
/// "smtkResourceTypeNameMacro" defined in Resource.h to declare this field in
/// tandem with uniqueName(). To relax the requirements of a) a macro definition
/// in a class header, or b) a mysterious constexpr in a resource declaration,
/// the free function name() will traverse one of two code paths to determine a
/// resource name.

namespace detail
{
// A compile-time test to check whether or not a class has a type_name defined.
template <typename T>
class is_named_resource
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

// The signature for our resource name-finding struct has two template
// parameters.
template <typename ResourceType, bool is_named>
struct resource_name;

// This partial template specialization deals with the case where
// <ResourceType> does not have a type_name. In this case, we create a temporary
/// resource and ask for its uniqueName.
template <typename ResourceType>
struct resource_name<ResourceType, false>
{
  static std::string name() { return ResourceType::create()->uniqueName(); }
};

// This partial template specialization deals with the case where
// <ResourceType> has a type_name. In this case, we can return the resource type
// name without instantiating the resource.
template <typename ResourceType>
struct resource_name<ResourceType, true>
{
  static std::string name() { return ResourceType::type_name; }
};
}

/// Return the name of a resource.
template <typename ResourceType>
std::string name()
{
  return detail::resource_name<ResourceType,
    detail::is_named_resource<ResourceType>::value>::name();
}
}
}

#endif // smtk_resource_Name_h
