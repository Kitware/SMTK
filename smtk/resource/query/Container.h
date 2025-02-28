//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_resource_query_Container_h
#define smtk_resource_query_Container_h

#include "smtk/CoreExports.h"

#include "smtk/resource/query/BadTypeError.h"

#include <memory>
#include <typeinfo>
#include <unordered_map>

namespace smtk
{
namespace resource
{
namespace query
{
/// A container for caching and retrieving instances of types that share a
/// common base class. Instances are retrieved using type information as a key,
/// allowing for simultaneous lookup and static downcast into that type. This
/// class differs from smtk::common::TypeContainer by the requirement that each
/// contained element have a common base class; this restriction enables us to
/// retrieve derived elements using base element indices as keys.
template<typename T>
class SMTK_ALWAYS_EXPORT Container
{
public:
  template<typename Type>
  bool contains() const
  {
    return (m_container.find(typeid(Type).hash_code()) != m_container.end());
  }

  bool contains(const std::size_t index) const
  {
    return (m_container.find(index) != m_container.end());
  }

  template<typename Type>
  const Type& get() const
  {
    return static_cast<Type&>(get(typeid(Type).hash_code()));
  }

  // If the input type is default constructible, we can always create a new
  // instance of it if it is not found. The method can therefore be marked
  // noexcept.
  template<typename Type>
  typename std::enable_if<std::is_default_constructible<Type>::value, Type&>::type get() noexcept
  {
    auto search = m_container.find(typeid(Type).hash_code());
    if (search == m_container.end())
    {
      search =
        m_container.emplace(std::make_pair(typeid(Type).hash_code(), std::make_unique<Type>()))
          .first;
    }

    return static_cast<Type&>(*(search->second));
  }

  // If the input type is not default constructible, we throw an error that
  // The type is not in the container.
  template<typename Type>
  typename std::enable_if<!std::is_default_constructible<Type>::value, Type&>::type get()
  {
    auto search = m_container.find(typeid(Type).hash_code());
    if (search == m_container.end())
    {
      throw BadTypeError(smtk::common::typeName<T>());
    }

    return static_cast<Type&>(*(search->second));
  }

  const T& get(const std::size_t& index) const
  {
    auto search = m_container.find(index);
    if (search == m_container.end())
    {
      throw BadTypeError(smtk::common::typeName<T>());
    }

    return *(search->second);
  }

  T& get(const std::size_t& index)
  {
    auto search = m_container.find(index);
    if (search == m_container.end())
    {
      throw BadTypeError(smtk::common::typeName<T>());
    }

    return *(search->second);
  }

  const std::unordered_map<std::size_t, std::unique_ptr<T>>& data() const { return m_container; }
  std::unordered_map<std::size_t, std::unique_ptr<T>>& data() { return m_container; }

private:
  std::unordered_map<std::size_t, std::unique_ptr<T>> m_container;
};
} // namespace query
} // namespace resource
} // namespace smtk

#endif
