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

#include "smtk/task/json/Configurator.h"

#include "smtk/task/Adaptor.h"
#include "smtk/task/Port.h"
#include "smtk/task/Task.h"

#include "smtk/common/Managers.h"
#include "smtk/common/TypeName.h"
#include "smtk/common/UUID.h"

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
/// This is needed in order to serialize dependencies among tasks which
/// are stored as pointers that could, in theory, form a cycle.
class SMTKCORE_EXPORT Helper
{
public:
  /// Swizzle IDs are serializable substitutes for pointers.
  using SwizzleId = Configurator<Task>::SwizzleId;
  /// JSON data type
  using json = nlohmann::json;

  /// Destructor is public, but you shouldn't use it.
  ~Helper();

  /// Return the helper "singleton".
  ///
  /// The object returned is a per-thread instance
  /// at the top of a stack that may be altered using
  /// the pushInstance() and popInstance() methods.
  /// This allows nested deserializers to each have
  /// their own context that appears to be globally
  /// available.
  static Helper& instance();

  /// Push a new top-level helper instance on the local thread's stack.
  ///
  /// The returned \a Helper will have:
  /// + The same managers as the previous (if any) helper.
  /// + The \a parent task is assigned the ID 0.
  static Helper& pushInstance(
    smtk::task::Manager& taskManager,
    const smtk::common::Managers::Ptr& otherManagers);

  /// Push a new (non-top-level) helper instance on the local thread's stack.
  ///
  /// The returned \a Helper will have:
  /// + The same taskManager as the previous helper.
  /// + The same managers as the previous (if any) helper.
  /// + The \a parent task is assigned the ID 0.
  static Helper& pushInstance(smtk::task::Task* parent);

  /// Pop a helper instance off the local thread's stack.
  static void popInstance();

  /// Return the nesting level (i.e., the number of helper instances in the stack).
  ///
  /// The outermost helper will return 1 (assuming you have called instance() first).
  static std::size_t nestingDepth();

  /// Turn on/off mapping of JSON UUIDs to new, random UUIDs.
  ///
  /// This is used by EmplaceWorklet.
  /// Do not call this after any tasks/ports/adaptors have been
  /// swizzled by any of this helper's configurators.
  void setMapUUIDs(bool shouldMap);
  bool mapUUIDs() const { return m_mapUUIDs; }

  Manager& taskManager() { return *m_taskManager; }

  /// Return an object for registering task classes and serialization helpers.
  Configurator<Task>& tasks();

  /// Return an object for registering port classes and serialization helpers.
  Configurator<Port>& ports();

  /// Return an object for registering adaptor classes and serialization helpers.
  Configurator<Adaptor>& adaptors();

  /// Set/get the managers to use when serializing/deserializing.
  ///
  /// Call setManagers() with an instance of all your application's
  /// managers before attempting to serialize/deserialize as helpers
  /// are allowed to use managers as needed.
  void setManagers(const smtk::common::Managers::Ptr& managers);
  smtk::common::Managers::Ptr managers();

  /// Reset the helper's state.
  ///
  /// This should be called before beginning serialization or deserialization.
  /// Additionally, calling it after each of these tasks is recommended since
  /// it will free memory.
  void clear();

  // --- Below here are methods specific to tasks and/or adaptors that
  // --- don't fit in the Configurator class or use Configurator-specific API.

  /// Populate \a tasks with the set of current tasks.
  void currentTasks(std::vector<Task*>& tasks);

  /// Populate \a ports with the set of current ports.
  void currentPorts(std::vector<Port*>& ports);

  /// Populate \a adaptors with the set of current adaptors.
  void currentAdaptors(std::vector<Adaptor*>& adaptors);

  /// Return a serialization of task-references that is consistent within
  /// the scope of serializing a set of tasks.
  json swizzleDependencies(const Task::PassedDependencies& deps);

  /// Return a deserialization of de-swizzled task-references.
  Task::PassedDependencies unswizzleDependencies(const json& ids) const;

  /// Returns true if the helper is for deserializing top-level or child tasks.
  bool topLevel() const { return m_topLevel; }

  /// Set/clear a pair of task IDs used when deserializing a single adaptor.
  ///
  /// Storing these IDs as state in the helper allows `from_json()` to
  /// be "stateless" (i.e., taking no additional parameters and using
  /// the Helper as a side effect).
  void setAdaptorTaskIds(SwizzleId fromId, SwizzleId toId);
  void setAdaptorTaskIds(const smtk::common::UUID& fromId, const smtk::common::UUID& toId);
  void clearAdaptorTaskIds();
  /// Get task-pointers based on the IDs set earlier.
  std::pair<Task*, Task*> getAdaptorTasks();

  /// Set the current helper's active task and all parents which shared the
  /// same task manager. You may pass (or expect) a null task.
  void setActiveSerializedTask(Task* task);
  Task* activeSerializedTask() const;

  /// Return a pointer to a persistent object given a JSON \a spec.
  ///
  /// Because workflow designers may provide worklets whose objects (tasks/ports/adaptors)
  /// do not have UUIDs, we need a way for worklet state to reference these objects.
  /// We must also handle cases where UUIDs *are* provided. This method is used by
  /// deserialization code to fetch the relevant object given a variety of input JSON
  /// that uniquely specifies objects in different ways.
  ///
  /// The \a spec may be:
  /// + a JSON array holding a pair of UUIDs: a resource UUID and a component UUID.
  ///   If the resource UUID is null, the component UUID is assumed to belong to
  ///   the current project.
  /// + a single integer: the swizzle ID of a task, port, or adaptor (as indicated by
  ///   the optional \a objType argument.
  /// + a JSON array holding a string and an integer: the string indicates the type
  ///   of component (one of "task", "port", or "adaptor") and the integer represents
  ///   a swizzle ID.
  smtk::resource::PersistentObject* objectFromJSONSpec(
    const json& spec,
    smtk::string::Token objType = "task");
  template<typename T>
  T* objectFromJSONSpecAs(const json& spec, smtk::string::Token objType = "task")
  {
    auto* obj = this->objectFromJSONSpec(spec, objType);
    auto* result = dynamic_cast<T*>(obj);
    return result;
  }

protected:
  Helper();
  Helper(Manager*);
  Manager* m_taskManager{ nullptr };
  Configurator<Task> m_tasks;
  Configurator<Port> m_ports;
  Configurator<Adaptor> m_adaptors;
  Task* m_activeSerializedTask{ nullptr };
  smtk::common::Managers::Ptr m_managers;
  SwizzleId m_adaptorFromId = ~static_cast<SwizzleId>(0);
  SwizzleId m_adaptorToId = ~static_cast<SwizzleId>(0);
  smtk::common::UUID m_adaptorFromUID;
  smtk::common::UUID m_adaptorToUID;
  /// m_topLevel indicates whether pushInstance() (false) or instance() (true)
  /// was used to create this helper. If m_topLevel is false, the parent task
  /// is assigned swizzle ID 1.
  bool m_topLevel{ true };
  /// m_mapUUIDs is true when deserializing worklet JSON.
  /// This causes all persistent objects in the JSON to have their UUIDs remapped.
  bool m_mapUUIDs{ false };
};

} // namespace json
} // namespace task
} // namespace smtk

// Include the configurator implementation here since
// it requires the Helper class defined.
#include "smtk/task/json/Configurator.txx"

#endif // smtk_task_json_Helper_h
