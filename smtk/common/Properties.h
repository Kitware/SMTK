//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_common_Properties_h
#define smtk_common_Properties_h

#include "smtk/CoreExports.h"

#include "smtk/SystemConfig.h"

#include "smtk/common/CompilerInformation.h"
#include "smtk/common/TypeName.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "nlohmann/json.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>

namespace smtk
{
namespace common
{
/// A common base class for properties of a given type. The PropertiesContainer
/// class is a collection of instances of PropertiesOfType, necessitating a
/// common base for their aggregate storage. An API for serialization is
/// included to avoid having to upcast to a templated type.
class SMTKCORE_EXPORT PropertiesBase
{
public:
  virtual ~PropertiesBase() {}

  virtual void to_json(nlohmann::json&) const {}
  virtual void from_json(const nlohmann::json&) {}
};

class Properties;

/// A specialization of the PropertiesContainer for a single type.
/// PropertiesOfType provides a non-templated API for accessing property
/// information, as well as serialization logic if the underlying type is
/// serializable.
template <typename Type>
class PropertiesOfType : public PropertiesBase
{
  friend class Properties;

protected:
  /// Construction of this class is delegated to Properties.
  PropertiesOfType() {}

public:
  typedef std::string key_type;
  typedef Type mapped_type;

  /// As an extension of PropertiesContainer's API, copy & move construction are
  /// not supported.
  PropertiesOfType(const PropertiesOfType&) = delete;
  PropertiesOfType(PropertiesOfType&&) = delete;
  PropertiesOfType& operator=(const PropertiesOfType&) = delete;
  PropertiesOfType& operator=(PropertiesOfType&&) = delete;

  /// Check whether a property associated with \a key is present.
  bool contains(const std::string& key) const { return (m_data.find(key) != m_data.end()); }

  /// Insert (\a key, \a value ) into the container.
  bool insert(const std::string& key, const Type& value)
  {
    return m_data.insert(std::make_pair(key, value)).second;
  }

  /// Emplace (\a key, \a value ) into the container.
  bool emplace(const std::string& key, Type&& value)
  {
    return m_data.emplace(std::make_pair(key, std::move(value))).second;
  }

  /// Erase property indexed by \a key from the container.
  void erase(const std::string& key) { m_data.erase(key); }

  /// Access property indexed by \a key.
  Type& operator[](const std::string& key) { return m_data[key]; }

  /// Access property indexed by \a key.
  Type& at(const std::string& key) { return m_data.at(key); }

  /// Access property indexed by \a key.
  const Type& at(const std::string& key) const { return m_data.at(key); }

  /// Access the class's underlying data.
  std::unordered_map<std::string, Type>& data() { return m_data; }
  const std::unordered_map<std::string, Type>& data() const { return m_data; }

  void to_json(nlohmann::json& j) const override { return to_json<Type>(j); }

  void from_json(const nlohmann::json& j) override { return from_json<Type>(j); }

private:
  template <typename T>
  typename std::enable_if<nlohmann::detail::is_compatible_type<nlohmann::json, T>::value>::type
  to_json(nlohmann::json& j) const
  {
    j = m_data;
  }

  template <typename T>
  typename std::enable_if<!nlohmann::detail::is_compatible_type<nlohmann::json, T>::value>::type
  to_json(nlohmann::json&) const
  {
  }

  template <typename T>
  typename std::enable_if<nlohmann::detail::is_compatible_type<nlohmann::json, T>::value>::type
  from_json(const nlohmann::json& j)
  {
    m_data = j.get<decltype(m_data)>();
  }

  template <typename T>
  typename std::enable_if<!nlohmann::detail::is_compatible_type<nlohmann::json, T>::value>::type
  from_json(const nlohmann::json&) const
  {
  }

  std::unordered_map<std::string, Type> m_data;
};

/// The PropertiesContainer class holds the storage and API for Properties. It
/// is decoupled from Properties to facilitate reuse by downstream classes that
/// can derive from PropertiesOfType<>. PropertiesContainer does not contain any
/// methods to insert types (this functionality is delegated to the downstream
/// Container class).
class SMTKCORE_EXPORT PropertiesContainer
{
public:
  virtual ~PropertiesContainer() = 0;

  /// Check whether a property of type \a Type associated with \a key is present.
  /// On average, this method has constant complexity and can therefore be used
  /// in conjunction with at() for conditional queries.
  template <typename Type>
  bool contains(const std::string& key) const
  {
    auto it = m_data.find(smtk::common::typeName<Type>());
    if (it == m_data.end())
    {
      return false;
    }

    return static_cast<const PropertiesOfType<Type>&>(*it->second).contains(key);
  }

  /// Insert (\a Type, \a key, \a value ) into the container.
  template <typename Type>
  bool insert(const std::string& key, const Type& value)
  {
    return get<Type>().insert(key, value);
  }

  /// Emplace (\a Type, \a key, \a value ) into the container.
  template <typename Type>
  bool emplace(const std::string& key, Type&& value)
  {
    return get<Type>().emplace(key, std::move(value));
  }

  /// Erase property of type \a Type indexed by \a key from the container.
  template <typename Type>
  void erase(const std::string& key)
  {
    auto& property = get<Type>();
    property.erase(key);
  }

  /// Access property of type \a Type indexed by \a key.
  /// On average, this method has constant complexity and can therefore be used
  /// in conjunction with contains() for conditional queries.
  template <typename Type>
  Type& at(const std::string& key)
  {
    return get<Type>().at(key);
  }

  /// Access property of type \a Type indexed by \a key.
  /// On average, this method has constant complexity and can therefore be used
  /// in conjunction with contains() for conditional queries.
  template <typename Type>
  const Type& at(const std::string& key) const
  {
    return get<Type>().at(key);
  }

  /// Access properties of type \a Type.
  template <typename Type>
  PropertiesOfType<Type>& get()
  {
    std::string key = smtk::common::typeName<Type>();
    auto it = m_data.find(key);
    if (it == m_data.end())
    {
      throw std::domain_error("No property with given type");
    }

    return static_cast<PropertiesOfType<Type>&>(*it->second);
  }

  /// Access properties of type \a Type.
  template <typename Type>
  const PropertiesOfType<Type>& get() const
  {
    auto it = m_data.find(smtk::common::typeName<Type>());
    if (it == m_data.end())
    {
      throw std::domain_error("No property with given type");
    }

    return static_cast<const PropertiesOfType<Type>&>(*it->second);
  }

  /// Check whether property type \a Type is supported.
  template <typename Type>
  bool hasPropertyType() const
  {
    return (m_data.find(smtk::common::typeName<Type>()) != m_data.end());
  }

  /// Access the class's underlying data.
  std::unordered_map<std::string, PropertiesBase*>& data() { return m_data; }
  const std::unordered_map<std::string, PropertiesBase*>& data() const { return m_data; }

private:
  std::unordered_map<std::string, PropertiesBase*> m_data;
};

inline PropertiesContainer::~PropertiesContainer()
{
  for (auto& pair : m_data)
  {
    delete pair.second;
  }
}

/// Properties is a generalized container for storing and accessing data using a
/// std::string key. The value type is open-ended and extensible; to accommodate
/// this flexibility, there is both a templated API on the PropertiesContainer
/// class and a specialized PropertiesOfType interface. Properties augments
/// PropertiesContainer with the ability to declare supported types; this
/// functionality is only exposed at construction to enforce RAII (otherwise,
/// serialization routines would have to bind type names to types).
class SMTKCORE_EXPORT Properties : public PropertiesContainer
{
public:
  Properties() {}

  template <typename List>
  Properties()
  {
    insertPropertyTypes<List>();
  }

  template <typename List>
  Properties(identity<List>)
  {
    insertPropertyTypes<List>();
  }

  ~Properties() {}

protected:
  template <typename Type>
  void insertPropertyType()
  {
    std::string key = smtk::common::typeName<Type>();
    auto it = data().find(key);
    if (it == data().end())
    {
      it = data().emplace(std::make_pair(key, new PropertiesOfType<Type>)).first;
    }
  }

  template <typename Tuple>
  void insertPropertyTypes()
  {
    Properties::insertPropertyTypes<0, Tuple>();
  }

private:
  template <std::size_t I, typename Tuple>
  inline typename std::enable_if<I != std::tuple_size<Tuple>::value>::type insertPropertyTypes()
  {
    this->insertPropertyType<typename std::tuple_element<I, Tuple>::type>();
    Properties::insertPropertyTypes<I + 1, Tuple>();
  }

  template <std::size_t I, typename Tuple>
  inline typename std::enable_if<I == std::tuple_size<Tuple>::value>::type insertPropertyTypes()
  {
  }
};
}
}

#endif
