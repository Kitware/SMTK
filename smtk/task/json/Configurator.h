//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_json_Configurator_h
#define smtk_task_json_Configurator_h

#include "smtk/task/Task.h"

#include "smtk/common/Managers.h"
#include "smtk/common/TypeName.h"

#include "nlohmann/json.hpp"

#include <exception>
#include <functional>
#include <string>
#include <thread>

namespace smtk
{
namespace task
{
namespace json
{

class Helper;

SMTKCORE_EXPORT std::mutex& typeMutex();

typedef std::mutex& (*TypeMutexFunction)();

/// A helper for serializing task configurations.
///
/// This is needed in order to serialized dependencies among tasks which
/// are stored as pointers that could, in theory, form a cycle.
template<typename ObjectType, TypeMutexFunction = &smtk::task::json::typeMutex>
class SMTKCORE_EXPORT Configurator
{
public:
  /// Methods that can produce a configuration for a task have this signature.
  using ConfigurationHelper =
    std::function<typename ObjectType::Configuration(const ObjectType*, Helper&)>;
  /// How ConfigurationHelper functions are stored.
  ///
  /// Keys are task class-names; values are functors that produce a JSON
  /// object given a task of that type.
  using HelperTypeMap = std::unordered_map<std::string, ConfigurationHelper>;
  /// JSON data type
  using json = nlohmann::json;

  // Construct a configurator. The helper you pass must not be null.
  Configurator(Helper* helper);
  ~Configurator() = default;

  ///@{
  /// Methods used in registrars to register/unregister types.
  ///
  /// These are piggybacked onto the task-manager instance registration (i.e.,
  /// called within the Registrar's method accepting an smtk::task::Manager),
  /// so a Schwarz counter is not required to ensure these are only called
  /// when needed. See smtk::task::Registrar for an example of how to use
  /// these methods.
  ///
  /// Also, because serialization and deserialization are inherently a
  /// run-time activity, we don't make an attempt at compile-time type-safety.
  template<typename ClassList, typename HelperList>
  inline static bool registerTypes();

  template<typename ClassList>
  inline static bool unregisterTypes();

  template<std::size_t I, typename ClassList, class HelperList>
  inline static typename std::enable_if<I != std::tuple_size<ClassList>::value, bool>::type
  registerTypes();

  template<std::size_t I, typename ClassList, class HelperList>
  inline static typename std::enable_if<I == std::tuple_size<ClassList>::value, bool>::type
  registerTypes();

  template<std::size_t I, typename ClassList>
  inline static typename std::enable_if<I != std::tuple_size<ClassList>::value, bool>::type
  unregisterTypes();

  template<std::size_t I, typename ClassList>
  inline static typename std::enable_if<I == std::tuple_size<ClassList>::value, bool>::type
  unregisterTypes();
  ///@}

  inline static bool registerType(const std::string& typeName, ConfigurationHelper helper);
  inline static bool unregisterType(const std::string& typeName);

  /// Return json configuration for the given object using registered helpers.
  typename ObjectType::Configuration configuration(const ObjectType* object);

  /// Reset the helper's state.
  ///
  /// This should be called before beginning serialization or deserialization.
  /// Additionally, calling it after each of these tasks is recommended since
  /// it will free memory.
  void clear();

  /// Return the ID of an object as computed by the swizzler.
  /// This will allocate a new ID if none exists.
  std::size_t swizzleId(const ObjectType* object);

  /// Return the pointer to an object given its swizzled ID (or null).
  ObjectType* unswizzle(std::size_t objectId) const;

  /// Return a serialization of task-references that is consistent within
  /// the scope of serializing a set of tasks.
  // json swizzleDependencies(const ObjectType::PassedDependencies& deps);

  /// Return a deserialization of de-swizzled task-references.
  // ObjectType::PassedDependencies unswizzleDependencies(const json& ids) const;

protected:
  Helper* m_helper;
  std::unordered_map<ObjectType*, std::size_t> m_swizzleFwd;
  std::unordered_map<std::size_t, ObjectType*> m_swizzleBck;
  std::size_t m_nextSwizzle = 1;
  static HelperTypeMap s_types;
};

} // namespace json
} // namespace task
} // namespace smtk

#endif // smtk_task_json_Configurator_h
