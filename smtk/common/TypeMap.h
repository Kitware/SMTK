//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_common_TypeMap_h
#define smtk_common_TypeMap_h

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
/// A common base class for properties of a given type. The TypeMapContainer
/// class is a collection of instances of TypeMapEntry, necessitating a
/// common base for their aggregate storage. An API for serialization is
/// included to avoid having to upcast to a templated type.
class SMTKCORE_EXPORT TypeMapEntryBase
{
public:
  virtual ~TypeMapEntryBase() = default;

  virtual void to_json(nlohmann::json&) const {}
  virtual void from_json(const nlohmann::json&) {}

  virtual void clear() {}
  virtual std::size_t size() const { return 0; }
};

template<typename KeyType>
class TypeMap;

/// A specialization of the TypeMapBase for a single type.
/// TypeMapEntry provides a non-templated API for accessing
/// information, as well as serialization logic if the underlying type is
/// serializable.
template<typename KeyType, typename Type>
class SMTK_ALWAYS_EXPORT TypeMapEntry : public TypeMapEntryBase
{
  friend class TypeMap<KeyType>;

protected:
  /// Construction of this class is delegated to TypeMap.
  TypeMapEntry() = default;

public:
  typedef KeyType key_type;
  typedef Type mapped_type;

  /// As an extension of TypeMapBase's API, copy & move construction are
  /// not supported.
  TypeMapEntry(const TypeMapEntry&) = delete;
  TypeMapEntry(TypeMapEntry&&) = delete;
  TypeMapEntry& operator=(const TypeMapEntry&) = delete;
  TypeMapEntry& operator=(TypeMapEntry&&) = delete;

  /// Check whether a value associated with \a key is present.
  bool contains(const KeyType& key) const { return (m_data.find(key) != m_data.end()); }

  /// Insert (\a key, \a value ) into the map.
  bool insert(const KeyType& key, const Type& value)
  {
    return m_data.insert(std::make_pair(key, value)).second;
  }

  /// Emplace (\a key, \a value ) into the map.
  bool emplace(const KeyType& key, Type&& value)
  {
    return m_data.emplace(std::make_pair(key, std::move(value))).second;
  }

  /// Erase value indexed by \a key from the map.
  void erase(const KeyType& key) { m_data.erase(key); }

  /// Access value indexed by \a key.
  Type& operator[](const KeyType& key) { return m_data[key]; }

  /// Access value indexed by \a key.
  Type& at(const KeyType& key) { return m_data.at(key); }

  /// Access value indexed by \a key.
  const Type& at(const KeyType& key) const { return m_data.at(key); }

  /// Access the class's underlying data.
  std::unordered_map<KeyType, Type>& data() { return m_data; }
  const std::unordered_map<KeyType, Type>& data() const { return m_data; }

  std::size_t size() const override { return m_data.size(); }
  void clear() override { m_data.clear(); }

  void to_json(nlohmann::json& j) const override { return to_json<Type>(j); }

  void from_json(const nlohmann::json& j) override { return from_json<Type>(j); }

private:
  template<typename T>
  typename std::enable_if<nlohmann::detail::is_compatible_type<nlohmann::json, T>::value>::type
  to_json(nlohmann::json& j) const
  {
    j = m_data;
  }

  template<typename T>
  typename std::enable_if<!nlohmann::detail::is_compatible_type<nlohmann::json, T>::value>::type
  to_json(nlohmann::json&) const
  {
  }

  template<typename T>
  typename std::enable_if<nlohmann::detail::is_compatible_type<nlohmann::json, T>::value>::type
  from_json(const nlohmann::json& j)
  {
    m_data = j.get<decltype(m_data)>();
  }

  template<typename T>
  typename std::enable_if<!nlohmann::detail::is_compatible_type<nlohmann::json, T>::value>::type
  from_json(const nlohmann::json&) const
  {
  }

  std::unordered_map<KeyType, Type> m_data;
};

/// The TypeMapBase class holds the storage and API for TypeMap. It
/// is decoupled from TypeMap to facilitate reuse by downstream classes that
/// can derive from TypeMapEntry<>. TypeMapBase does not contain any
/// methods to insert types (this functionality is delegated to the downstream
/// TypeMap class).
template<typename KeyType>
class SMTK_ALWAYS_EXPORT TypeMapBase
{
public:
  virtual ~TypeMapBase() = 0;

  /// Check whether a value of type \a Type associated with \a key is present.
  /// On average, this method has constant complexity and can therefore be used
  /// in conjunction with at() for conditional queries.
  template<typename Type>
  bool contains(const KeyType& key) const
  {
    auto it = m_data.find(smtk::common::typeName<Type>());
    if (it == m_data.end())
    {
      return false;
    }

    return static_cast<const TypeMapEntry<KeyType, Type>&>(*it->second).contains(key);
  }

  /// Insert (\a Type, \a key, \a value ) into the map.
  template<typename Type>
  bool insert(const KeyType& key, const Type& value)
  {
    return get<Type>().insert(key, value);
  }

  /// Emplace (\a Type, \a key, \a value ) into the map.
  template<typename Type>
  bool emplace(const KeyType& key, Type&& value)
  {
    return get<Type>().emplace(key, std::forward<Type>(value));
  }

  /// Erase value of type \a Type indexed by \a key from the map.
  template<typename Type>
  void erase(const KeyType& key)
  {
    auto& property = get<Type>();
    property.erase(key);
  }

  /// Access value of type \a Type indexed by \a key.
  /// On average, this method has constant complexity and can therefore be used
  /// in conjunction with contains() for conditional queries.
  template<typename Type>
  Type& at(const KeyType& key)
  {
    return get<Type>().at(key);
  }

  /// Access value of type \a Type indexed by \a key.
  /// On average, this method has constant complexity and can therefore be used
  /// in conjunction with contains() for conditional queries.
  template<typename Type>
  const Type& at(const KeyType& key) const
  {
    return get<Type>().at(key);
  }

  /// Access values of type \a Type.
  template<typename Type>
  TypeMapEntry<KeyType, Type>& get()
  {
    std::string name = smtk::common::typeName<Type>();
    auto it = m_data.find(name);
    if (it == m_data.end())
    {
      throw std::domain_error("No entry with given type");
    }

    return static_cast<TypeMapEntry<KeyType, Type>&>(*it->second);
  }

  /// Access values of type \a Type.
  template<typename Type>
  const TypeMapEntry<KeyType, Type>& get() const
  {
    auto it = m_data.find(smtk::common::typeName<Type>());
    if (it == m_data.end())
    {
      throw std::domain_error("No entry with given type");
    }

    return static_cast<const TypeMapEntry<KeyType, Type>&>(*it->second);
  }

  /// Check whether type \a Type is supported.
  template<typename Type>
  bool containsType() const
  {
    return (m_data.find(smtk::common::typeName<Type>()) != m_data.end());
  }

  /// Access the class's underlying data.
  std::unordered_map<std::string, TypeMapEntryBase*>& data() { return m_data; }
  const std::unordered_map<std::string, TypeMapEntryBase*>& data() const { return m_data; }

private:
  std::unordered_map<std::string, TypeMapEntryBase*> m_data;
};

template<typename KeyType>
inline TypeMapBase<KeyType>::~TypeMapBase()
{
  for (auto& pair : m_data)
  {
    delete pair.second;
  }
}

/// TypeMap is a generalized map for storing and accessing data using a
/// key. The value type is open-ended and extensible; to accommodate this
/// flexibility, there is both a templated API on the TypeMapBase
/// class and a specialized TypeMapEntry interface. TypeMap augments
/// TypeMapBase with the ability to declare supported types; this
/// functionality is only exposed at construction to enforce RAII (otherwise,
/// serialization routines would have to bind type names to types).
template<typename KeyType = std::string>
class SMTK_ALWAYS_EXPORT TypeMap : public TypeMapBase<KeyType>
{
public:
  typedef KeyType key_type;

  // MSVC sees `= default` as a duplicate of the template version.
  // NOLINTNEXTLINE(modernize-use-equals-default)
  TypeMap() {}

  template<typename List>
  TypeMap()
  {
    insertTypes<List>();
  }

  template<typename List>
  TypeMap(identity<List>)
  {
    insertTypes<List>();
  }

  ~TypeMap() override = default;

protected:
  template<typename Type>
  void insertType()
  {
    std::string key = smtk::common::typeName<Type>();
    auto it = TypeMapBase<KeyType>::data().find(key);
    if (it == TypeMapBase<KeyType>::data().end())
    {
      it = TypeMapBase<KeyType>::data()
             .emplace(std::make_pair(key, new TypeMapEntry<key_type, Type>))
             .first;
    }
  }

  template<typename Tuple>
  void insertTypes()
  {
    TypeMap::insertTypes<0, Tuple>();
  }

private:
  template<std::size_t I, typename Tuple>
  typename std::enable_if<I != std::tuple_size<Tuple>::value>::type insertTypes()
  {
    this->insertType<typename std::tuple_element<I, Tuple>::type>();
    TypeMap::insertTypes<I + 1, Tuple>();
  }

  template<std::size_t I, typename Tuple>
  typename std::enable_if<I == std::tuple_size<Tuple>::value>::type insertTypes()
  {
  }
};
} // namespace common
} // namespace smtk

#endif
