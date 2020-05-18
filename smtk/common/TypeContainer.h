//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_common_TypeContainer_h
#define smtk_common_TypeContainer_h

#include "smtk/CoreExports.h"

#include "smtk/SystemConfig.h"

#include "smtk/common/CompilerInformation.h"
#include "smtk/common/TypeName.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <unordered_map>

namespace smtk
{
namespace common
{
/// A container for caching and retrieving instances of types. Instances are
/// retrieved using type information as a key, allowing for simultaneous lookup
/// and static downcast into that type. TypeContainer supports copying by cloning
/// its elements using thier copy constructors.
class SMTKCORE_EXPORT TypeContainer
{
  struct Wrapper
  {
    virtual ~Wrapper() = default;
    virtual std::unique_ptr<Wrapper> clone() const = 0;
  };

  template<typename Type>
  struct WrapperFor : Wrapper
  {
    template<typename... Args>
    WrapperFor(Args&&... v)
      : value(std::forward<Args>(v)...)
    {
    }

    std::unique_ptr<Wrapper> clone() const
    {
#ifdef SMTK_HAVE_CXX_14
      return std::make_unique<WrapperFor<Type>>(std::make_unique<Type>(*value));
#else
      return std::unique_ptr<Wrapper>(new WrapperFor<Type>(new Type(*value)));
#endif
    }

    std::unique_ptr<Type> value;
  };

public:
  class BadTypeError : public std::out_of_range
  {
  public:
    BadTypeError(const std::string& typeName)
      : std::out_of_range("Type \"" + typeName + "\" not available in this container")
    {
    }
  };

  /// Construct an empty TypeContainer.
  TypeContainer() = default;

  /// Construct a TypeContainer whose contents are copied from an existing
  /// TypeContainer.
  TypeContainer(const TypeContainer&);

  /// Move the contents of one TypeContainer into a new TypeContainer.
  TypeContainer(TypeContainer&&) = default;

  /// Copy the contents of an existing TypeContainer into this one.
  TypeContainer& operator=(const TypeContainer&);

  /// Move the contents of an existing TypeContainer into this one.
  TypeContainer& operator=(TypeContainer&&) = default;

  /// Construct a TypeContainer instance from any number of elements. Elements
  /// are added in the order they appear in the constructor, so subsequent values
  /// for the same type will be ignored.
  template<
    typename Arg,
    typename... Args,
    typename std::enable_if<!std::is_base_of<TypeContainer, Arg>::value, int>::type = 0>
  TypeContainer(const Arg& arg, const Args&... args)
  {
    insertAll(arg, args...);
  }

  virtual ~TypeContainer();

  /// Check if a Type is present in the TypeContainer.
  template<typename Type>
  bool contains() const
  {
    return (m_container.find(typeid(Type).hash_code()) != m_container.end());
  }

  /// Insert a Type instance into the TypeContainer.
  template<typename Type>
  bool insert(const Type& value)
  {
    return m_container
      .emplace(std::make_pair(
        typeid(Type).hash_code(),
#ifdef SMTK_HAVE_CXX_14
        std::make_unique<WrapperFor<Type>>(std::make_unique<Type>(value))))
#else
        std::unique_ptr<Wrapper>(new WrapperFor<Type>(std::unique_ptr<Type>(new Type((value)))))))
#endif
      .second;
  }

  /// Emplace a Type instance into the TypeContainer.
  template<typename Type, typename... Args>
  bool emplace(Args&&... args)
  {
    return m_container
      .emplace(std::make_pair(
        typeid(Type).hash_code(),
#ifdef SMTK_HAVE_CXX_14
        std::make_unique<WrapperFor<Type>>(std::make_unique<Type>(std::forward<Args>(args)...))))
#else
        std::unique_ptr<Wrapper>(
          new WrapperFor<Type>(std::unique_ptr<Type>(new Type(std::forward<Args>(args)...))))))
#endif
      .second;
  }

  /// Access a Type instance, and throw if it is not in the TypeContainer.
  template<typename Type>
  const Type& get() const
  {
    auto search = m_container.find(typeid(Type).hash_code());
    if (search == m_container.end())
    {
      throw BadTypeError(smtk::common::typeName<Type>());
    }

    return *(static_cast<WrapperFor<Type>*>(search->second.get()))->value;
  }

  /// For default-constructible types, access a Type instance, creating one if it
  /// is not in the TypeContainer.
  template<typename Type>
  typename std::enable_if<std::is_default_constructible<Type>::value, Type&>::type get() noexcept
  {
    auto search = m_container.find(typeid(Type).hash_code());
    if (search == m_container.end())
    {
      search = m_container
                 .emplace(std::make_pair(
                   typeid(Type).hash_code(),
#ifdef SMTK_HAVE_CXX_14
                   std::make_unique<WrapperFor<Type>>(std::make_unique<Type>())))
#else
                   std::unique_ptr<Wrapper>(new WrapperFor<Type>(std::unique_ptr<Type>(new Type)))))
#endif
                 .first;
    }

    return *(static_cast<WrapperFor<Type>*>(search->second.get()))->value;
  }

  /// For non-default-constructible types, access a Type instance; throw if it is
  /// not in the TypeContainer.
  template<typename Type>
  typename std::enable_if<!std::is_default_constructible<Type>::value, Type&>::type get()
  {
    auto search = m_container.find(typeid(Type).hash_code());
    if (search == m_container.end())
    {
      throw BadTypeError(smtk::common::typeName<Type>());
    }

    return *(static_cast<WrapperFor<Type>*>(search->second.get()))->value;
  }

  template<typename Type>
  bool erase()
  {
    return m_container.erase(typeid(Type).hash_code()) > 0;
  }

  bool empty() const noexcept { return m_container.empty(); }

  std::size_t size() const noexcept { return m_container.size(); }

  void clear() noexcept { m_container.clear(); }

private:
  template<typename Arg, typename... Args>
  typename std::enable_if<!std::is_base_of<TypeContainer, Arg>::value, bool>::type insertAll(
    const Arg& arg,
    const Args&... args)
  {
    return insert<Arg>(arg) && insertAll(args...);
  }
  bool insertAll() { return true; }

  std::unordered_map<std::size_t, std::unique_ptr<Wrapper>> m_container;
};
} // namespace common
} // namespace smtk

#endif
