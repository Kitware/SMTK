//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_resource_Properties_h
#define smtk_resource_Properties_h

#include "smtk/CoreExports.h"
#include "smtk/common/TypeMap.h"
#include "smtk/common/UUID.h"
#include "smtk/string/Token.h"

#include "smtk/common/json/jsonUUID.h"
#include "smtk/string/json/jsonToken.h"

#include "smtk/resource/json/jsonPropertyCoordinateFrame.h"
#include "smtk/resource/properties/CoordinateFrame.h"

#include <algorithm>

namespace smtk
{
namespace resource
{
/// The Properties classes defined in the resource namespace use the TypeMap
/// classes defined in smtk::common, but provide a distinct API to transpose the
/// UUID and property key for storage vs presentation; this is to prevent the
/// creation of a map for each component within a resource.
namespace detail
{
/// The Properties classes defined in this namespace closely correlate to
/// counterparts in smtk::common::TypeMap, but are tailored to provide a UUID as
/// an additional lookup parameter.
///
/// A common base class for resource properties. It is used to provide an API
/// for removing properties associated with removed IDs to avoid having to
/// upcast to a templated type.
class SMTKCORE_EXPORT PropertiesBase
{
public:
  virtual ~PropertiesBase() = default;

  /// Erase all properties held by this object which correspond to the given \a uid.
  ///
  /// The number of entries removed is returned.
  virtual std::size_t eraseId(const smtk::common::UUID& uid) = 0;

  /// Return true if properties of this type may be copy-constructed.
  virtual bool isCopyable() const = 0;

  /// Copy the property or properties held by \a other which correspond to \a otherId
  /// into this object under the given \a uid.
  ///
  /// If \a propertyName is invalid, then all properties are copied. Otherwise,
  /// just the named property is copied (assuming it exists in \a other).
  ///
  /// The number of entries copied is returned.
  virtual std::size_t copyFrom(
    PropertiesBase* other,
    smtk::string::Token propertyName,
    const smtk::common::UUID& otherId,
    const smtk::common::UUID& uid) = 0;

  /// Insert (into \a ids) all of the UUIDs that properties held by this object reference.
  virtual void allIds(std::set<smtk::common::UUID>& ids) const = 0;
  /// Insert (into \a names) all of the property names that exist in this object.
  virtual void allNames(std::unordered_set<smtk::string::Token>& names) const = 0;
  /// Insert (into \a ids) all of the UUIDs for which \a propName is defined.
  virtual void idsWithProperty(smtk::string::Token& propName, std::set<smtk::common::UUID>& ids)
    const = 0;
};

template<typename Type>
class PropertiesOfType;

/// A specialization of the smtk::common::PropertiesContainer for a single type.
/// PropertiesOfType provides a non-templated API for accessing property
/// information, as well as logic for erasing properties associated with a UUID
/// (needed for cleaning up after components are deleted).
template<typename Type>
class PropertiesOfType<std::unordered_map<smtk::common::UUID, Type>>
  : public smtk::common::TypeMapEntry<std::string, std::unordered_map<smtk::common::UUID, Type>>
  , public PropertiesBase
{
  friend class Properties;
  using SelfType = PropertiesOfType<std::unordered_map<smtk::common::UUID, Type>>;

  PropertiesOfType()
    : smtk::common::TypeMapEntry<std::string, std::unordered_map<smtk::common::UUID, Type>>()
    , PropertiesBase()
  {
  }

  struct IsCopyable
  {
    template<typename PropType = Type>
    typename std::enable_if<!std::is_copy_constructible<PropType>::value, bool>::type operator()()
      const
    {
      return false;
    }
    template<typename PropType = Type>
    typename std::enable_if<std::is_copy_constructible<PropType>::value, bool>::type operator()()
      const
    {
      return true;
    }
  };

  struct Copier
  {
    template<typename PropType = Type>
    typename std::enable_if<!std::is_copy_constructible<PropType>::value, std::size_t>::type
    operator()(
      SelfType* self,
      PropertiesBase* otherBase,
      smtk::string::Token propertyName,
      const smtk::common::UUID& otherId,
      const smtk::common::UUID& uid) const
    {
      (void)self;
      (void)propertyName;
      (void)otherBase;
      (void)otherId;
      (void)uid;
      return 0;
    }

    template<typename PropType = Type>
    typename std::enable_if<std::is_copy_constructible<PropType>::value, std::size_t>::type
    operator()(
      SelfType* self,
      PropertiesBase* otherBase,
      smtk::string::Token propertyName,
      const smtk::common::UUID& otherId,
      const smtk::common::UUID& uid) const
    {
      std::size_t numCopied = 0;
      auto* other =
        dynamic_cast<PropertiesOfType<std::unordered_map<smtk::common::UUID, PropType>>*>(
          otherBase);
      if (!other || !uid || !otherId)
      {
        return numCopied;
      }
      if (propertyName.valid())
      {
        // Copy the one property (if it exists).
        auto nameIt = other->data().find(propertyName.data());
        if (nameIt == other->data().end())
        {
          return numCopied;
        }
        auto valIt = nameIt->second.find(otherId);
        if (valIt == nameIt->second.end())
        {
          return numCopied;
        }
        ++numCopied;
        self->data()[nameIt->first][uid] = valIt->second;
      }
      else
      {
        // Copy all the properties that exist.
        for (const auto& pair : other->data())
        {
          auto fromIt = pair.second.find(otherId);
          if (fromIt != pair.second.end())
          {
            ++numCopied; // += fromIt->second.size();
            self->data()[pair.first][uid] = fromIt->second;
          }
        }
      }
      return numCopied;
    }
  };

public:
  std::size_t eraseId(const smtk::common::UUID& id) override
  {
    std::size_t count = 0;
    for (auto& pair : this->data())
    {
      count += pair.second.erase(id);
    }
    return count;
  }

  bool isCopyable() const override { return std::is_copy_constructible<Type>::value; }

  std::size_t copyFrom(
    PropertiesBase* otherBase,
    smtk::string::Token propertyName,
    const smtk::common::UUID& otherId,
    const smtk::common::UUID& uid) override
  {
    return Copier()(this, otherBase, propertyName, otherId, uid);
  }

  void allNames(std::unordered_set<smtk::string::Token>& names) const override
  {
    for (const auto& nameToCompToValue : this->data())
    {
      names.insert(nameToCompToValue.first);
    }
  }

  void allIds(std::set<smtk::common::UUID>& ids) const override
  {
    for (const auto& nameToCompToValue : this->data())
    {
      for (const auto& compToValue : nameToCompToValue.second)
      {
        ids.insert(compToValue.first);
      }
    }
  }

  void idsWithProperty(smtk::string::Token& propName, std::set<smtk::common::UUID>& ids)
    const override
  {
    auto nameIt = this->data().find(propName.data());
    if (nameIt == this->data().end())
    {
      return;
    }
    for (const auto& compToValue : nameIt->second)
    {
      ids.insert(compToValue.first);
    }
  }
};

/// Properties is a generalized container for storing and accessing data using a
/// std::string key. This Properties differs from smtk::common::TypeMapBase
/// by constructing custom TypeMapEntries<> that are tailored for use with
/// additional UUID indexing.
class SMTKCORE_EXPORT Properties : public smtk::common::TypeMapBase<std::string>
{
public:
  // MSVC sees `= default` as a duplicate of the template version.
  // NOLINTNEXTLINE(modernize-use-equals-default)
  Properties() {}

  template<typename List>
  Properties()
  {
    insertPropertyTypes<List>();
  }

  template<typename List>
  Properties(identity<List>)
  {
    insertPropertyTypes<List>();
  }

  std::size_t eraseId(const smtk::common::UUID& id)
  {
    std::size_t count = 0;
    auto& map = this->data();
    for (auto& pair : map)
    {
      count += dynamic_cast<PropertiesBase*>(pair.second)->eraseId(id);
    }
    return count;
  }

  template<typename Type>
  std::size_t eraseIdForType(const smtk::common::UUID& id)
  {
    return dynamic_cast<PropertiesBase&>(this->get<Type>()).eraseId(id);
  }

  // TODO: Putting the following two methods in the public API breaks RAII.
  // There needs to be a way for a derived resource to augment its types of
  // properties, though.
  template<typename Type>
  void insertPropertyType()
  {
    std::string key = smtk::common::typeName<Type>();
    auto it = data().find(key);
    if (it == data().end())
    {
      it = data().emplace(std::make_pair(key, new detail::PropertiesOfType<Type>)).first;
    }
  }

  template<typename Tuple>
  void insertPropertyTypes()
  {
    Properties::insertPropertyTypes<0, Tuple>();
  }

private:
  template<std::size_t I, typename Tuple>
  inline typename std::enable_if<I != std::tuple_size<Tuple>::value>::type insertPropertyTypes()
  {
    this->insertPropertyType<typename std::tuple_element<I, Tuple>::type>();
    Properties::insertPropertyTypes<I + 1, Tuple>();
  }

  template<std::size_t I, typename Tuple>
  inline typename std::enable_if<I == std::tuple_size<Tuple>::value>::type insertPropertyTypes()
  {
  }
};
} // namespace detail

class Component;
class Resource;
class Properties;

/// A specialization of the Properties container for a single type.
/// ConstPropertiesOfType provides a non-templated API for accessing property
/// information.
template<typename Type>
class ConstPropertiesOfType
{
  using IndexedType = std::unordered_map<smtk::common::UUID, Type>;

  friend class Properties;
  ConstPropertiesOfType(
    const smtk::common::UUID& id,
    const detail::PropertiesOfType<IndexedType>& properties)
    : m_id(id)
    , m_properties(properties)
  {
  }

public:
  /// Check whether a property associated with \a key is present.
  bool contains(const std::string& key) const
  {
    if (!m_properties.contains(key))
    {
      return false;
    }
    const std::unordered_map<smtk::common::UUID, Type>& data = get(key);
    return (data.find(m_id) != data.end());
  }

  /// Access property indexed by \a key.
  const Type& at(const std::string& key) const { return get(key).at(m_id); }

  /// Check if any properties of this type are associated with m_id.
  bool empty() const
  {
    using PropertyDataMap =
      std::unordered_map<std::string, std::unordered_map<smtk::common::UUID, Type>>;
    const PropertyDataMap& data = m_properties.data();
    return std::all_of(
      data.begin(), data.end(), [this](const typename PropertyDataMap::value_type& pair) {
        return pair.second.find(m_id) == pair.second.end();
      });
  }

  std::set<std::string> keys() const
  {
    std::set<std::string> keys;
    for (auto& pair : m_properties.data())
    {
      if (pair.second.find(m_id) != pair.second.end())
      {
        keys.insert(pair.first);
      }
    }
    return keys;
  }

private:
  const std::unordered_map<smtk::common::UUID, Type>& get(const std::string& key) const
  {
    return m_properties.at(key);
  }

  const smtk::common::UUID& m_id;
  const detail::PropertiesOfType<IndexedType>& m_properties;
};

/// A specialization of the Properties container for a single type.
/// PropertiesOfType provides a non-templated API for accessing property
/// information.
template<typename Type>
class PropertiesOfType
{
  using IndexedType = std::unordered_map<smtk::common::UUID, Type>;

  friend class Properties;
  PropertiesOfType(const smtk::common::UUID& id, detail::PropertiesOfType<IndexedType>& properties)
    : m_id(id)
    , m_properties(properties)
  {
  }

public:
  /// Check whether a property associated with \a key is present.
  bool contains(const std::string& key) const
  {
    if (!m_properties.contains(key))
    {
      return false;
    }
    const std::unordered_map<smtk::common::UUID, Type>& data = get(key);
    return (data.find(m_id) != data.end());
  }

  /// Insert (\a key, \a value ) into the container.
  bool insert(const std::string& key, const Type& value)
  {
    return get(key).insert(std::make_pair(m_id, value)).second;
  }

  /// Emplace (\a key, \a value ) into the container.
  bool emplace(const std::string& key, Type&& value)
  {
    return get(key).emplace(std::make_pair(m_id, std::move(value))).second;
  }

  /// Erase property indexed by \a key from the container.
  void erase(const std::string& key)
  {
    get(key).erase(m_id);
    if (get(key).empty())
    {
      m_properties.erase(key);
    }
  }

  /// Access property indexed by \a key.
  Type& operator[](const std::string& key) { return get(key)[m_id]; }

  /// Access property indexed by \a key.
  Type& at(const std::string& key) { return get(key).at(m_id); }

  /// Access property indexed by \a key.
  const Type& at(const std::string& key) const { return get(key).at(m_id); }

  /// Check if any properties of this type are associated with m_id.
  bool empty() const
  {
    for (auto& pair : m_properties.data())
    {
      if (pair.second.find(m_id) != pair.second.end())
      {
        return false;
      }
    }
    return true;
  }

  /// Remove all properties of this type corresponding to our ID.
  ///
  /// The returned value is the number of removed property entries.
  std::size_t clear()
  {
    std::size_t count = m_properties.eraseId(m_id);
    return count;
  }

  std::set<std::string> keys() const
  {
    std::set<std::string> keys;
    for (auto& pair : m_properties.data())
    {
      if (pair.second.find(m_id) != pair.second.end())
      {
        keys.insert(pair.first);
      }
    }
    return keys;
  }

private:
  const std::unordered_map<smtk::common::UUID, Type>& get(const std::string& key) const
  {
    return m_properties.at(key);
  }
  std::unordered_map<smtk::common::UUID, Type>& get(const std::string& key)
  {
    if (!m_properties.contains(key))
    {
      m_properties.emplace(key, typename detail::PropertiesOfType<IndexedType>::mapped_type());
    }
    return m_properties.at(key);
  }

  const smtk::common::UUID& m_id;
  detail::PropertiesOfType<IndexedType>& m_properties;
};

/// Resource/Component properties store data as maps from UUIDs to values and
/// present data as key/value pairs on Resources/Components themselves. This
/// misdirection is necessary to avoid the construction and storage of a map for
/// each component in a resource. This virtual class provides a uniform API for
/// both Resources and Components; it is subclassed for the respective class to
/// utilize a single container per Resource.
class SMTKCORE_EXPORT Properties
{
public:
  template<typename Type>
  using Indexed = std::unordered_map<smtk::common::UUID, Type>;

  /// Check whether a property associated with \a key is present.
  template<typename Type>
  bool contains(const std::string& key) const
  {
    return get<Type>().contains(key);
  }

  /// Insert (\a key, \a value ) into the container.
  template<typename Type>
  bool insert(const std::string& key, const Type& value)
  {
    return get<Type>().insert(key, value);
  }

  /// Emplace (\a key, \a value ) into the container.
  template<typename Type>
  bool emplace(const std::string& key, Type&& value)
  {
    return get<Type>().emplace(key, std::forward<Type>(value));
  }

  /// Erase property indexed by \a key from the container.
  template<typename Type>
  void erase(const std::string& key)
  {
    return get<Type>().erase(key);
  }

  /// Access property indexed by \a key.
  template<typename Type>
  Type& at(const std::string& key)
  {
    return get<Type>().at(key);
  }

  /// Access property indexed by \a key.
  template<typename Type>
  const Type& at(const std::string& key) const
  {
    return get<Type>().at(key);
  }

  /// Access properties of type \a Type.
  template<typename Type>
  PropertiesOfType<Type> get()
  {
    return PropertiesOfType<Type>(
      id(),
      static_cast<detail::PropertiesOfType<Indexed<Type>>&>(properties().get<Indexed<Type>>()));
  }

  /// Access properties of type \a Type.
  template<typename Type>
  const ConstPropertiesOfType<Type> get() const
  {
    return ConstPropertiesOfType<Type>(
      id(),
      static_cast<const detail::PropertiesOfType<Indexed<Type>>&>(
        properties().get<Indexed<Type>>()));
  }

  /// Remove all properties this object manages.
  virtual std::size_t clear() = 0;

  /// Return the set of property storage types that have been registered.
  std::unordered_set<smtk::string::Token> types() const
  {
    std::unordered_set<smtk::string::Token> result;
    const auto& tmap = this->properties().data();
    for (const auto& entry : tmap)
    {
      result.insert(entry.first);
    }
    return result;
  }

  /// Return the set of property names that exist for the given storage type.
  std::unordered_set<smtk::string::Token> namesForType(smtk::string::Token storageType) const
  {
    std::unordered_set<smtk::string::Token> result;
    const auto& tmap = this->properties().data();
    auto it = tmap.find(storageType.data());
    if (it == tmap.end())
    {
      return result;
    }
    auto* baseData = dynamic_cast<detail::PropertiesBase*>(it->second);
    if (baseData)
    {
      baseData->allNames(result);
    }
    return result;
  }

private:
  virtual const smtk::common::UUID& id() const = 0;
  virtual smtk::common::TypeMapBase<std::string>& properties() = 0;
  virtual const smtk::common::TypeMapBase<std::string>& properties() const = 0;

  struct EraseProperties
  {
    template<typename Data>
    void operator()(Data propertiesOfType, const smtk::common::UUID& uid, std::size_t& count)
    {
      std::cout << "Erase " << uid << " from " << propertiesOfType.keys().size() << "\n";
      count += propertiesOfType.clear();
    }
  };

  /**\brief Invoke a \a Functor (which accepts \a args) on each property type in the \a Tuple.
    *
    * For each property type, the functor is passed the input arguments (which it may
    * modify).
    */
  //@{
  template<typename Tuple, typename Functor, typename... Args>
  void invoke(Args&&... args) const
  {
    smtk::resource::Properties::invokeFunctors<0, Tuple, Functor>(std::forward<Args>(args)...);
  }

  template<typename Tuple, typename Functor, typename... Args>
  void invoke(Args&&... args)
  {
    smtk::resource::Properties::invokeFunctors<0, Tuple, Functor>(std::forward<Args>(args)...);
  }
  //@}

  // const version
  template<typename Entry, typename Functor, typename... Args>
  void invokeFunctor(Args&&... args) const
  {
    Functor f;
    f(std::forward<Args>(args)...);
  }

  // non-const version
  template<typename Entry, typename Functor, typename... Args>
  void invokeFunctor(Args&&... args)
  {
    Functor f;
    f(std::forward<Args>(args)...);
  }

  // const versions
  template<std::size_t I, typename Tuple, typename Functor, typename... Args>
  inline typename std::enable_if<I != std::tuple_size<Tuple>::value>::type invokeFunctors(
    Args&&... args) const
  {
    this->invokeFunctor<typename std::tuple_element<I, Tuple>::type, Functor>(
      // First argument is always PropertiesOfType<tuple_element<I, Tuple>>:
      this->template get<typename std::tuple_element<I, Tuple>::type>(),
      // Subsequent arguments are whatever is passed in:
      std::forward<Args>(args)...);
    smtk::resource::Properties::invokeFunctors<I + 1, Tuple, Functor>(std::forward<Args>(args)...);
  }

  // non-const version
  template<std::size_t I, typename Tuple, typename Functor, typename... Args>
  inline typename std::enable_if<I != std::tuple_size<Tuple>::value>::type invokeFunctors(
    Args&&... args)
  {
    this->invokeFunctor<typename std::tuple_element<I, Tuple>::type, Functor>(
      // First argument is always PropertiesOfType<tuple_element<I, Tuple>>:
      this->template get<typename std::tuple_element<I, Tuple>::type>(),
      // Subsequent arguments are whatever is passed in:
      std::forward<Args>(args)...);
    smtk::resource::Properties::invokeFunctors<I + 1, Tuple, Functor>(std::forward<Args>(args)...);
  }

  // This only needs a const version.
  template<std::size_t I, typename Tuple, typename Functor, typename... Args>
  inline typename std::enable_if<I == std::tuple_size<Tuple>::value>::type invokeFunctors(
    Args&&...) const
  {
  }

protected:
  /// This is a method subclasses may call from their implementation of clear()
  /// in order to iterate over all acceptable property types and remove a UUID
  /// from the map.
  template<typename PropertyTypeTuple>
  std::size_t clearInternal(const smtk::common::UUID& uid)
  {
    std::size_t count = 0;
    // auto& propertiesByTypeThenId = this->properties();
    this->invoke<PropertyTypeTuple, EraseProperties>(uid, count);
    return count;
  }
};

namespace detail
{
/// This specialization of smtk::resource::Properties completes aforementioned
/// class's implementation by holding a customization of
/// smtk::common::Properties as described in the above detail namespace.
class SMTKCORE_EXPORT ResourceProperties : public smtk::resource::Properties
{
  friend Resource;

  typedef detail::Properties ResourcePropertiesData;

public:
  /// The default value types for all resources and components are
  /// + int, double, string, and vectors of these types;
  /// + string tokens (integer hashes into a map producing strings) and sets of string tokens;
  /// + CoordinateFrame values plus maps from strings to CoordinateFrames.
  typedef std::tuple<
    Indexed<bool>,
    Indexed<int>,
    Indexed<long>,
    Indexed<double>,
    Indexed<std::string>,
    Indexed<smtk::string::Token>,
    Indexed<std::set<smtk::string::Token>>,
    Indexed<std::set<int>>,
    Indexed<std::vector<bool>>,
    Indexed<std::vector<int>>,
    Indexed<std::vector<long>>,
    Indexed<std::vector<double>>,
    Indexed<std::vector<std::string>>,
    Indexed<smtk::resource::properties::CoordinateFrame>,
    Indexed<std::map<std::string, smtk::resource::properties::CoordinateFrame>>>
    PropertyTypes;

  ResourcePropertiesData& data() { return m_data; }
  const ResourcePropertiesData& data() const { return m_data; }

  // TODO: Putting the following method in the public API breaks RAII. There
  // needs to be a way for a derived resource to augment its types of
  // properties, though.
  template<typename Type>
  void insertPropertyType()
  {
    m_data.insertPropertyType<Indexed<Type>>();
  }

  // Remove all properties on the resource
  std::size_t clear() override
  {
    std::size_t count = this->clearInternal<PropertyTypes>(this->id());
    return count;
  }

private:
  ResourceProperties(Resource* resource);

  const smtk::common::UUID& id() const override;
  smtk::common::TypeMapBase<std::string>& properties() override { return m_data; }
  const smtk::common::TypeMapBase<std::string>& properties() const override { return m_data; }

  Resource* m_resource;
  ResourcePropertiesData m_data;
};

/// This specialization of smtk::resource::Properties completes aforementioned
/// class's implementation by accessing the properties container held by the
/// component's resource.
class SMTKCORE_EXPORT ComponentProperties : public smtk::resource::Properties
{
public:
  // Remove all properties on the resource
  std::size_t clear() override
  {
    std::size_t count = this->clearInternal<ResourceProperties::PropertyTypes>(this->id());
    return count;
  }

protected:
  friend Component;

  ComponentProperties(const Component* component);
  ~ComponentProperties();

  const smtk::common::UUID& id() const override;
  smtk::common::TypeMapBase<std::string>& properties() override;
  const smtk::common::TypeMapBase<std::string>& properties() const override;

  const Component* m_component;
};
} // namespace detail
} // namespace resource
} // namespace smtk

#endif
