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

  /// Reset the configurator's state.
  ///
  /// This should be called before beginning serialization or deserialization.
  /// Additionally, calling it after each of these tasks is recommended since
  /// it will free memory.
  void clear();

  /// Reset just the portion of the configurator's state related to negative SwizzleIds.
  ///
  /// Sometimes serializers need to process nested objects.
  /// While a more general solution is possible (wherein each nested
  /// call to the serializer creates a new helper and pops it once
  /// complete, it is simpler to constrain nested swizzlers to never
  /// overlap one another. Then, we can simply re-use the namespace
  /// of negative swizzle IDs for each nested call.
  /// This method clears any negative swizzle IDs and should be called
  /// at the start of each nested deserialization.
  void clearNestedSwizzles();

  /// Return the ID of an object as computed by the swizzler.
  /// This will allocate a new, negative ID if none exists.
  SwizzleId nestedSwizzleId(const ObjectType* object);

  /// Return the ID of an object as computed by the swizzler.
  /// This will allocate a new ID if none exists.
  SwizzleId swizzleId(const ObjectType* object);

  /// When deserializing an object, we have the swizzle ID assigned previously.
  /// Accept the given ID; if it already exists, then return false and print
  /// a warning. Otherwise, assign the given ID and return true.
  bool setSwizzleId(const ObjectType* object, SwizzleId swizzle);

  /// Return the pointer to an object given its swizzled ID (or null).
  ObjectType* unswizzle(SwizzleId objectId) const;

  /// Populate the vector with tasks whose swizzle ID is \a start or above.
  void currentObjects(std::vector<ObjectType*>& objects, SwizzleId start = 2);

protected:
  Helper* m_helper;
  std::unordered_map<ObjectType*, SwizzleId> m_swizzleFwd;
  std::map<SwizzleId, ObjectType*> m_swizzleBck;
  SwizzleId m_nextSwizzle = 1;
  SwizzleId m_nextNested = -1;
  static HelperTypeMap s_types;
};

#ifdef SMTK_MSVC
// MSVC requires explicit template instantiations to be exported. Otherwise,
// multiple instantiations will occur for each consuming DLL which will cause
// issues with type containers since each instance of what is effectively the
// same type will have a different type ID in each DLL.
template class SMTKCORE_EXPORT Configurator<Task>;
template class SMTKCORE_EXPORT Configurator<Adaptor>;
#endif

} // namespace json
} // namespace task
} // namespace smtk

#endif // smtk_task_json_Configurator_h
