//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_json_Configurator_txx
#define smtk_task_json_Configurator_txx

#include "smtk/task/json/Configurator.h"
#include "smtk/task/json/Helper.h"

#include "smtk/io/Logger.h"

namespace smtk
{
namespace task
{
namespace json
{

template<typename ObjectType, TypeMutexFunction MF>
Configurator<ObjectType, MF>::Configurator(Helper* helper)
  : m_helper(helper)
{
}

template<typename ObjectType, TypeMutexFunction MF>
template<typename ClassList, typename HelperList>
inline bool Configurator<ObjectType, MF>::registerTypes()
{
  static_assert(
    std::tuple_size<ClassList>::value == std::tuple_size<HelperList>::value,
    "Class and helper tuples must be of same length.");
  return registerTypes<0, ClassList, HelperList>();
}

template<typename ObjectType, TypeMutexFunction MF>
template<typename ClassList>
inline bool Configurator<ObjectType, MF>::unregisterTypes()
{
  return unregisterTypes<0, ClassList>();
}

template<typename ObjectType, TypeMutexFunction MF>
template<std::size_t I, typename ClassList, class HelperList>
inline typename std::enable_if<I != std::tuple_size<ClassList>::value, bool>::type
Configurator<ObjectType, MF>::registerTypes()
{
  auto typeName = smtk::common::typeName<typename std::tuple_element<I, ClassList>::type>();
  using HelperType = typename std::tuple_element<I, HelperList>::type;
  HelperType helper;
  bool registered = Configurator<ObjectType, MF>::registerType(typeName, helper);
  return registered && registerTypes<I + 1, ClassList, HelperList>();
}

template<typename ObjectType, TypeMutexFunction MF>
template<std::size_t I, typename ClassList, class HelperList>
inline typename std::enable_if<I == std::tuple_size<ClassList>::value, bool>::type
Configurator<ObjectType, MF>::registerTypes()
{
  return true;
}

template<typename ObjectType, TypeMutexFunction MF>
template<std::size_t I, typename ClassList>
inline typename std::enable_if<I != std::tuple_size<ClassList>::value, bool>::type
Configurator<ObjectType, MF>::unregisterTypes()
{
  auto typeName = smtk::common::typeName<typename std::tuple_element<I, ClassList>::type>();
  bool unregistered = Configurator<ObjectType, MF>::unregisterType(typeName);
  return unregistered && unregisterTypes<I + 1, ClassList>();
}

template<typename ObjectType, TypeMutexFunction MF>
template<std::size_t I, typename ClassList>
inline typename std::enable_if<I == std::tuple_size<ClassList>::value, bool>::type
Configurator<ObjectType, MF>::unregisterTypes()
{
  return true;
}

template<typename ObjectType, TypeMutexFunction MF>
inline bool Configurator<ObjectType, MF>::registerType(
  const std::string& typeName,
  ConfigurationHelper helper)
{
  std::lock_guard<std::mutex> lock(MF());
  return s_types.insert({ typeName, helper }).second;
}

template<typename ObjectType, TypeMutexFunction MF>
inline bool Configurator<ObjectType, MF>::unregisterType(const std::string& typeName)
{
  std::lock_guard<std::mutex> lock(MF());
  return s_types.erase(typeName) > 0;
}

template<typename ObjectType, TypeMutexFunction MF>
/// Return json configuration for the given object using registered helpers.
typename ObjectType::Configuration Configurator<ObjectType, MF>::configuration(
  const ObjectType* object)
{
  typename ObjectType::Configuration config;
  if (object)
  {
    auto typeName = object->typeName();
    ConfigurationHelper objectHelper = nullptr;
    typename HelperTypeMap::const_iterator it;
    {
      std::lock_guard<std::mutex> lock(MF());
      it = s_types.find(typeName);
      if (it == s_types.end())
      {
        return config;
      }
      objectHelper = it->second;
    }
    this->swizzleId(object); // Assign an object ID as early as possible.
    config = objectHelper(object, *m_helper);
  }
  return config;
}

/// Reset the helper's state.
///
/// This should be called before beginning serialization or deserialization.
/// Additionally, calling it after each of these tasks is recommended since
/// it will free memory.
template<typename ObjectType, TypeMutexFunction MF>
void Configurator<ObjectType, MF>::clear()
{
  m_swizzleFwd.clear();
  m_swizzleBck.clear();
  m_nextSwizzle = 1;
}

/// Return the ID of an object as computed by the swizzler.
/// This will allocate a new ID if none exists.
template<typename ObjectType, TypeMutexFunction MF>
typename Configurator<ObjectType, MF>::SwizzleId Configurator<ObjectType, MF>::swizzleId(
  const ObjectType* object)
{
  if (!object)
  {
    return 0;
  }
  auto ncptr = std::const_pointer_cast<ObjectType>(
    std::dynamic_pointer_cast<const ObjectType>(object->shared_from_this()));
  return this->swizzleId(ncptr);
}

/// Return the ID of an object as computed by the swizzler.
/// This will allocate a new ID if none exists.
template<typename ObjectType, TypeMutexFunction MF>
typename Configurator<ObjectType, MF>::SwizzleId Configurator<ObjectType, MF>::swizzleId(
  const typename ObjectType::Ptr& object)
{
  if (!object)
  {
    return 0;
  }
  const auto& it = m_swizzleFwd.find(object);
  if (it != m_swizzleFwd.end())
  {
    return it->second;
  }
  SwizzleId id = m_nextSwizzle++;
  m_swizzleFwd[object] = id;
  m_swizzleBck[id] = object.get();
  return id;
}

/// Assign a previously-provided swizzle ID to the object.
/// This will warn and return false if the ID already exists.
template<typename ObjectType, TypeMutexFunction MF>
bool Configurator<ObjectType, MF>::setSwizzleId(
  const ObjectType* object,
  typename Configurator<ObjectType, MF>::SwizzleId swizzle)
{
  if (!object)
  {
    return false;
  }
  auto bit = m_swizzleBck.find(swizzle);
  if (bit != m_swizzleBck.end())
  {
    smtkWarningMacro(
      smtk::io::Logger::instance(),
      "Deserialized swizzle ID " << swizzle << " is already assigned to"
                                 << "\"" << bit->second->name() << "\" " << bit->second
                                 << ". Skipping.");
    return false;
  }
  // Need a non-const ObjectType in some cases:
  auto ncptr = std::const_pointer_cast<ObjectType>(
    std::dynamic_pointer_cast<const ObjectType>(object->shared_from_this()));
  m_swizzleFwd[ncptr] = swizzle;
  m_swizzleBck[swizzle] = ncptr.get();
  if (swizzle >= m_nextSwizzle)
  {
    m_nextSwizzle = swizzle + 1;
  }
  return true;
}

/// Assign a previously-provided swizzle ID to the object.
/// This will warn and return false if the ID already exists.
template<typename ObjectType, TypeMutexFunction MF>
bool Configurator<ObjectType, MF>::setSwizzleId(
  const typename ObjectType::Ptr& object,
  typename Configurator<ObjectType, MF>::SwizzleId swizzle)
{
  if (!object)
  {
    return false;
  }
  auto bit = m_swizzleBck.find(swizzle);
  if (bit != m_swizzleBck.end())
  {
    smtkWarningMacro(
      smtk::io::Logger::instance(),
      "Deserialized swizzle ID " << swizzle << " is already assigned to"
                                 << "\"" << bit->second->name() << "\" " << bit->second
                                 << ". Skipping.");
    return false;
  }
  m_swizzleFwd[object] = swizzle;
  m_swizzleBck[swizzle] = object.get();
  if (swizzle >= m_nextSwizzle)
  {
    m_nextSwizzle = swizzle + 1;
  }
  return true;
}

/// Return the pointer to an object given its swizzled ID (or null).
template<typename ObjectType, TypeMutexFunction MF>
ObjectType* Configurator<ObjectType, MF>::unswizzle(SwizzleId objectId) const
{
  auto it = m_swizzleBck.find(objectId);
  if (it == m_swizzleBck.end())
  {
    return nullptr;
  }
  return it->second;
}

template<typename ObjectType, TypeMutexFunction MF>
void Configurator<ObjectType, MF>::currentObjects(
  std::vector<ObjectType*>& objects,
  SwizzleId start)
{
  objects.clear();
  for (const auto& entry : m_swizzleBck)
  {
    if (entry.first >= start)
    {
      objects.push_back(entry.second);
    }
  }
}

template<typename ObjectType, TypeMutexFunction MF>
ObjectType* Configurator<ObjectType, MF>::construct(nlohmann::json dict)
{
  SwizzleId swizzleId = 0;
  smtk::common::UUID objId = smtk::common::UUID::null();
  smtk::common::UUID origId = smtk::common::UUID::null();
  typename ObjectType::Ptr obj(nullptr);
  auto idit = dict.find("id");
  if (idit != dict.end())
  {
    if (idit->is_number_integer())
    {
      swizzleId = idit->get<SwizzleId>();
    }
    else if (idit->is_string())
    {
      objId = idit->get<smtk::common::UUID>();
      if (m_helper->mapUUIDs())
      {
        origId = objId;
        // Write a new UUID to the JSON so that when we deserialize below
        // the object is unique relative to the JSON source.
        objId = smtk::common::UUID::random();
        *idit = objId;
      }

      // If a UUID is provided, also allow a swizzle ID separately in "swizzle":
      auto swit = dict.find("swizzle");
      if (swit != dict.end())
      {
        swizzleId = swit->get<SwizzleId>();
      }
    }
    // Deserialize the object via nlohmann::json, using from_json().
    // If m_helper->mapUUIDs() is true, its UUID will not match
    // objId obtained above.
    obj = dict;
    if (swizzleId)
    {
      this->setSwizzleId(obj, swizzleId);
      objId = obj->id();
    }
    else
    {
      swizzleId = this->swizzleId(obj);
    }
    m_objectById[obj->id()] = obj.get();
    if (origId != obj->id())
    {
      m_idMap[origId] = obj->id();
    }
  }
  return obj.get();
}

template<typename ObjectType, TypeMutexFunction MF>
ObjectType* Configurator<ObjectType, MF>::get(const smtk::common::UUID& uid)
{
  auto oit = m_objectById.find(uid);
  if (oit == m_objectById.end())
  {
    auto iit = m_idMap.find(uid);
    if (iit != m_idMap.end())
    {
      oit = m_objectById.find(iit->second);
      if (oit == m_objectById.end())
      {
        return nullptr;
      }
    }
    else
    {
      return nullptr;
    }
  }
  return oit->second;
}

template<typename ObjectType, TypeMutexFunction MF>
ObjectType* Configurator<ObjectType, MF>::get(const nlohmann::json& spec)
{
  if (spec.contains("swizzle"))
  {
    return this->unswizzle(spec.at("swizzle").get<SwizzleId>());
  }
  if (spec.contains("id"))
  {
    auto jid = spec.at("id");
    if (jid.is_number_integer())
    {
      return this->unswizzle(jid.get<SwizzleId>());
    }
    else if (jid.is_string())
    {
      return this->get(jid.get<smtk::common::UUID>());
    }
  }
  return nullptr;
}

template<typename ObjectType, TypeMutexFunction MF>
typename Configurator<ObjectType, MF>::HelperTypeMap Configurator<ObjectType, MF>::s_types;

} // namespace json
} // namespace task
} // namespace smtk

#endif // smtk_task_json_Configurator_txx
