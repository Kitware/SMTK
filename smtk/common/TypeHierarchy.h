//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_common_TypeHierarchy_h
#define smtk_common_TypeHierarchy_h

#include "smtk/common/TypeName.h"
#include "smtk/string/Token.h"

namespace smtk
{
namespace common
{
/// @file TypeHierarchy.h \brief Compute a class hierarchy using developer-provided type-aliases.

namespace detail
{

// Used by typeHierarchy with parentClasses to produce a container holding inherited type names.
template<typename Container, typename StopAtType = void>
struct addNames
{
  addNames(Container& c)
    : m_container(c)
  {
  }

  template<typename T>
  bool operator()()
  {
    if (!std::is_same<StopAtType, T>::value)
    {
      std::string typeName = smtk::common::typeName<T>();
      m_container.insert(m_container.end(), typeName);
      return true;
    }
    return false;
  }
  Container& m_container;
};

} // namespace detail

///@{
/**
 * Determine whether the provided class (\a Class ) has a parent class.
 *
 * The \a Class template-parameter should be some class that uses the `smtkSuperclass()`
 * or `smtkTypeMacroBase()` to define a `Superclass` type-alias.
 * The `value` in this class is true when a Superclass type-alias exists and
 * false otherwise.
 */
template<typename Class>
class HasSuperclass
{
  class No
  {
  };
  class Yes
  {
    No no[2];
  };

  template<typename C>
  static Yes Test(typename C::Superclass*);
  template<typename C>
  static No Test(...);

public:
  enum
  {
    value = sizeof(Test<Class>(nullptr)) == sizeof(Yes)
  };
};
///@}

///@{
/**
 * Invoke a functor on the named type and each of its parent types.
 *
 * The \a Class template-parameter should be some class that uses the `smtkSuperclass()`
 * macro (or the `smtkTypeMacroBase()` if it has no superclasses) to define a
 * `Superclass` type-alias, as this is how the inheritance hierarchy is traversed.
 *
 * Call the static `enumerate()` method of parentClasses with a functor
 * that accepts no arguments and a single template parameter.
 * If the return value of your functor is void, then `enumerate()` will invoke
 * your functor once on every type in your object's hierarchy.
 * If the return value of your functor is a boolen, then `enumerate()` will
 * invoke your functor on every type in your object's hierarchy until the
 * functor returns false (indicating early termination is requested).
 * See detail::addNames() above for an example of the latter.
 */
template<typename Class, bool IsDerived = HasSuperclass<Class>::value>
struct parentClasses;

template<typename Class>
struct parentClasses<Class, false>
{
  template<typename Functor>
  static void enumerate(Functor& ff)
  {
    ff.template operator()<Class>();
  }
};

template<typename Class>
struct parentClasses<Class, true>
{
  template<typename Functor>
  static void enumerate(Functor& ff)
  {
    if (ff.template operator()<Class>())
    {
      parentClasses<typename Class::Superclass>::enumerate(ff);
    }
  }
};
///@}

///@{
/**
 * Populate the \a container with the name of this class and its ancestors.
 *
 * The \a Class template-parameter should be an object
 * that uses the `smtkTypeMacro()` to define a `Superclass` type-alias, as this
 * is how the inheritance hierarchy is traversed.
 *
 * The version of this function that accepts 2 template parameters
 * uses the second parameter to iterate over a partial hierarchy
 * truncated at (not including) the \a StopAtType.
 */
template<typename Class, typename Container>
void typeHierarchy(Container& container)
{
  detail::addNames<Container> addNames(container);
  parentClasses<Class>::enumerate(addNames);
}

template<typename Class, typename StopAtType, typename Container>
void typeHierarchy(Container& container)
{
  detail::addNames<Container, StopAtType> addNames(container);
  parentClasses<Class>::enumerate(addNames);
}
///@}

} // namespace common
} // namespace smtk

#endif // smtk_common_TypeHierarchy_h
