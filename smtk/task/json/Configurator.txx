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

template<typename ObjectType, TypeMutexFunction MF>
void Configurator<ObjectType, MF>::clearNestedSwizzles()
{
  std::unordered_set<ObjectType*> removals;
  auto end = m_swizzleBck.lower_bound(SwizzleId(0));
  for (auto it = m_swizzleBck.begin(); it != end; ++it)
  {
    removals.insert(it->second);
  }
  m_swizzleBck.erase(m_swizzleBck.begin(), end);
  for (const auto& object : removals)
  {
    m_swizzleFwd.erase(object);
  }
  m_nextNested = -1;
}

template<typename ObjectType, TypeMutexFunction MF>
typename Configurator<ObjectType, MF>::SwizzleId Configurator<ObjectType, MF>::nestedSwizzleId(
  const ObjectType* object)
{
  if (!object)
  {
    return 0;
  }
  auto* ncobject = const_cast<ObjectType*>(object); // Need a non-const ObjectType in some cases.
  const auto& it = m_swizzleFwd.find(ncobject);
  if (it != m_swizzleFwd.end())
  {
    return it->second;
  }
  SwizzleId id = m_nextNested--;
  m_swizzleFwd[ncobject] = id;
  m_swizzleBck[id] = ncobject;
  return id;
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
  auto* ncobject = const_cast<ObjectType*>(object); // Need a non-const ObjectType in some cases.
  const auto& it = m_swizzleFwd.find(ncobject);
  if (it != m_swizzleFwd.end())
  {
    return it->second;
  }
  SwizzleId id = m_nextSwizzle++;
  m_swizzleFwd[ncobject] = id;
  m_swizzleBck[id] = ncobject;
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
  auto* ncobject = const_cast<ObjectType*>(object); // Need a non-const ObjectType in some cases.
  m_swizzleFwd[ncobject] = swizzle;
  m_swizzleBck[swizzle] = ncobject;
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
typename Configurator<ObjectType, MF>::HelperTypeMap Configurator<ObjectType, MF>::s_types;

} // namespace json
} // namespace task
} // namespace smtk

#endif // smtk_task_json_Configurator_txx
