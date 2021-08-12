//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_json_Helper_h
#define smtk_task_json_Helper_h

#include "smtk/task/Task.h"

#include "smtk/common/Managers.h"
#include "smtk/common/TypeName.h"

#include <exception>
#include <string>

namespace smtk
{
namespace task
{
namespace json
{

/// A helper for serializing task configurations.
///
/// This is needed in order to serialized dependencies among tasks which
/// are stored as pointers that could, in theory, form a cycle.
class SMTKCORE_EXPORT Helper
{
public:
  /// Methods that can produce a configuration for a task have this signature.
  using ConfigurationHelper = std::function<Task::Configuration(const Task*, Helper&)>;
  /// How ConfigurationHelper functions are stored.
  ///
  /// Keys are task class-names; values are functors that produce a JSON
  /// object given a task of that type.
  using HelperTypeMap = std::unordered_map<std::string, ConfigurationHelper>;
  /// JSON data type
  using json = nlohmann::json;

  /// Destructor is public, but you shouldn't use it.
  ~Helper();

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
  static bool registerTypes()
  {
    static_assert(
      std::tuple_size<ClassList>::value == std::tuple_size<HelperList>::value,
      "Class and helper tuples must be of same length.");
    return registerTypes<0, ClassList, HelperList>();
  }
  template<typename ClassList>
  static bool unregisterTypes()
  {
    return unregisterTypes<0, ClassList>();
  }

  template<std::size_t I, typename ClassList, class HelperList>
  static typename std::enable_if<I != std::tuple_size<ClassList>::value, bool>::type registerTypes()
  {
    auto typeName = smtk::common::typeName<typename std::tuple_element<I, ClassList>::type>();
    using HelperType =
      typename std::tuple_element<I, HelperList>::type; // smtk::task::json::jsonTask;
    HelperType helper;
    bool registered = Helper::registerType(typeName, helper);
    return registered && registerTypes<I + 1, ClassList, HelperList>();
  }
  template<std::size_t I, typename ClassList, class HelperList>
  static typename std::enable_if<I == std::tuple_size<ClassList>::value, bool>::type registerTypes()
  {
    return true;
  }

  template<std::size_t I, typename ClassList>
  static typename std::enable_if<I != std::tuple_size<ClassList>::value, bool>::type
  unregisterTypes()
  {
    auto typeName = smtk::common::typeName<typename std::tuple_element<I, ClassList>::type>();
    bool unregistered = Helper::unregisterType(typeName);
    return unregistered && unregisterTypes<I + 1, ClassList>();
  }
  template<std::size_t I, typename ClassList>
  static typename std::enable_if<I == std::tuple_size<ClassList>::value, bool>::type
  unregisterTypes()
  {
    return true;
  }
  ///@}

  static bool registerType(const std::string& typeName, ConfigurationHelper helper);
  static bool unregisterType(const std::string& typeName);

  /// Return the helper singleton.
  static Helper& instance();

  /// Set/get the managers to use when serializing/deserializing.
  ///
  /// Call setManagers() with an instance of all your application's
  /// managers before attempting to serialize/deserialize as helpers
  /// are allowed to use managers as needed.
  void setManagers(const smtk::common::Managers::Ptr& managers);
  smtk::common::Managers::Ptr managers();

  /// Return json configuration for the given task using registered helpers.
  Task::Configuration configuration(const Task*);

  /// Reset the helper's state.
  ///
  /// This should be called before beginning serialization or deserialization.
  /// Additionally, calling it after each of these tasks is recommended since
  /// it will free memory.
  void clear();

  /// Return the ID of a task as computed by the swizzler.
  /// This will allocate a new ID if none exists.
  std::size_t swizzleId(const Task* task);

  /// Return a serialization of task-references that is consistent within
  /// the scope of serializing a set of tasks.
  json swizzleDependencies(const Task::PassedDependencies& deps);

  /// Return a deserialization of de-swizzled task-references.
  Task::PassedDependencies unswizzleDependencies(const json& ids) const;

protected:
  Helper();
  smtk::common::Managers::Ptr m_managers;
  std::unordered_map<Task*, std::size_t> m_swizzleFwd;
  std::unordered_map<std::size_t, Task*> m_swizzleBck;
  std::size_t m_nextSwizzle = 1;
  static HelperTypeMap s_types;
};

} // namespace json
} // namespace task
} // namespace smtk

#endif // smtk_task_json_Helper_h
