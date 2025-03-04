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

#include "smtk/task/Adaptor.h"
#include "smtk/task/Port.h"
#include "smtk/task/Task.h"

#include "smtk/task/json/jsonAdaptor.h"
#include "smtk/task/json/jsonPort.h"
#include "smtk/task/json/jsonTask.h"

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
class SMTK_ALWAYS_EXPORT Configurator
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
  /// Swizzle IDs are serializable substitutes for pointers.
  ///
  /// This type must be signed since negative IDs are used for children of
  /// a Group task.
  using SwizzleId = int;
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
  static bool registerTypes();

  template<typename ClassList>
  static bool unregisterTypes();

  template<std::size_t I, typename ClassList, class HelperList>
  static typename std::enable_if<I != std::tuple_size<ClassList>::value, bool>::type
  registerTypes();

  template<std::size_t I, typename ClassList, class HelperList>
  static typename std::enable_if<I == std::tuple_size<ClassList>::value, bool>::type
  registerTypes();

  template<std::size_t I, typename ClassList>
  static typename std::enable_if<I != std::tuple_size<ClassList>::value, bool>::type
  unregisterTypes();

  template<std::size_t I, typename ClassList>
  static typename std::enable_if<I == std::tuple_size<ClassList>::value, bool>::type
  unregisterTypes();
  ///@}

  static bool registerType(const std::string& typeName, ConfigurationHelper helper);
  static bool unregisterType(const std::string& typeName);

  /// Return json configuration for the given object using registered helpers.
  typename ObjectType::Configuration configuration(const ObjectType* object);

  /// Reset the configurator's state.
  ///
  /// This should be called before beginning serialization or deserialization.
  /// Additionally, calling it after each of these tasks is recommended since
  /// it will free memory.
  void clear();

  /// Return the ID of an object as computed by the swizzler.
  /// This will allocate a new ID if none exists.
  SwizzleId swizzleId(const ObjectType* object);
  SwizzleId swizzleId(const typename ObjectType::Ptr& object);

  /// When deserializing an object, we have the swizzle ID assigned previously.
  /// Accept the given ID; if it already exists, then return false and print
  /// a warning. Otherwise, assign the given ID and return true.
  bool setSwizzleId(const ObjectType* object, SwizzleId swizzle);
  bool setSwizzleId(const typename ObjectType::Ptr& object, SwizzleId swizzle);

  /// Return the pointer to an object given its swizzled ID (or null).
  ObjectType* unswizzle(SwizzleId objectId) const;

  /// Populate the vector with tasks whose swizzle ID is \a start or above.
  void currentObjects(std::vector<ObjectType*>& objects, SwizzleId start = 2);

  /// Return whether the current swizzling maps are both empty.
  bool empty() const { return m_swizzleFwd.empty() && m_swizzleBck.empty(); }

  /// Given a JSON \a dict containing configuration data for
  /// an object, construct one and map its "id" and "swizzle" field(s)
  /// if present.
  ///
  /// This method is used by deserialization code to create tasks,
  /// ports, and adaptors given their JSON specifications (\a dict).
  /// It indexes these objects so that they can be referenced elsewhere
  /// in the JSON by UUID, by swizzle ID, or by object type (as a string
  /// token) and swizzle ID.
  ///
  /// This method also accounts for the parent-helper's mapUUIDs setting.
  /// If true, a random UUID is assigned to each newly-encountered UUID and
  /// the mapping is stored in a separate map.
  /// This allows task::Worklet JSON to contain "temporary" UUIDs that are
  /// modified during the deserialization done by EmplaceWorklet or other
  /// operations.
  ///
  /// Note that we pass \a dict by value intentionally so that when
  /// the hepler is mapping UUIDs we can alter \a dict.
  ObjectType* construct(nlohmann::json dict);

  /// Look up an object instantiated via construct() by its UUID.
  ObjectType* get(const smtk::common::UUID& uid);
  /// Look up an object instantiated via construct() by its JSON (using "id" or "swizzle").
  ObjectType* get(const nlohmann::json& spec);

  /// Return the map to instance UUIDs given "mapped" UUIDs.
  ///
  /// This is used when the parent Helper's mapUUID flag is set
  /// to deserialized instances that do not have the same UUIDs
  /// as those specified in the JSON.
  const std::unordered_map<smtk::common::UUID, smtk::common::UUID>& uuidMap() { return m_idMap; }

protected:
  Helper* m_helper;
  std::unordered_map<typename ObjectType::Ptr, SwizzleId> m_swizzleFwd;
  std::map<SwizzleId, ObjectType*> m_swizzleBck;
  std::unordered_map<smtk::common::UUID, ObjectType*> m_objectById;
  std::unordered_map<smtk::common::UUID, smtk::common::UUID> m_idMap;
  SwizzleId m_nextSwizzle = 1;
  static HelperTypeMap s_types;
};

#ifdef SMTK_MSVC
// MSVC requires explicit template instantiations to be exported. Otherwise,
// multiple instantiations will occur for each consuming DLL which will cause
// issues with type containers since each instance of what is effectively the
// same type will have a different type ID in each DLL.
template class SMTKCORE_EXPORT Configurator<Task>;
template class SMTKCORE_EXPORT Configurator<Port>;
template class SMTKCORE_EXPORT Configurator<Adaptor>;
#endif

} // namespace json
} // namespace task
} // namespace smtk

#endif // smtk_task_json_Configurator_h
