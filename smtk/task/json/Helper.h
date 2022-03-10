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

  /// Push a new helper instance on the local thread's stack.
  ///
  /// The returned \a Helper will have:
  /// + The same managers as the previous (if any) helper.
  /// + The \a parent task is assigned the ID 0.
  static Helper& pushInstance(smtk::task::Task* parent);

  /// Pop a helper instance off the local thread's stack.
  static void popInstance();

  /// Return the nesting level (i.e., the number of helper instances in the stack).
  ///
  /// The outermost helper will return 1 (assuming you have called instance() first).
  static std::size_t nestingDepth();

  /// Return an object for registering task classes and serialization helpers.
  Configurator<Task>& tasks();

  /// Return an object for registering adaptor classes and serialization helpers.
  Configurator<Adaptor>& adaptors();

  /// Set/get the managers to use when serializing/deserializing.
  ///
  /// Call setManagers() with an instance of all your application's
  /// managers before attempting to serialize/deserialize as helpers
  /// are allowed to use managers as needed.
  void setManagers(const smtk::common::Managers::Ptr& managers);
  smtk::common::Managers::Ptr managers();

  /// Reset only negative task IDs.
  ///
  /// Negative task IDs are used to when deserializing a Group task's
  /// children. Because only a single Group is deserialized at a time
  /// (per thread), there are no namespace collisions.
  void clearGroupTaskIds();

  /// Reset the helper's state.
  ///
  /// This should be called before beginning serialization or deserialization.
  /// Additionally, calling it after each of these tasks is recommended since
  /// it will free memory.
  void clear();

  // --- Below here are methods specific to tasks and/or adaptors that
  // --- don't fit in the Configurator class.

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
  void clearAdaptorTaskIds();
  /// Get task-pointers based on the IDs set earlier.
  std::pair<Task*, Task*> getAdaptorTasks();

protected:
  Helper();
  Configurator<Task> m_tasks;
  Configurator<Adaptor> m_adaptors;
  smtk::common::Managers::Ptr m_managers;
  SwizzleId m_adaptorFromId = ~static_cast<SwizzleId>(0);
  SwizzleId m_adaptorToId = ~static_cast<SwizzleId>(0);
  /// m_topLevel indicates whether pushInstance() (false) or instance() (true)
  /// was used to create this helper. If m_topLevel is false, the parent task
  /// is assigned swizzle ID 1.
  bool m_topLevel = true;
};

} // namespace json
} // namespace task
} // namespace smtk

// Include the configurator implementation here since
// it requires the Helper class defined.
#include "smtk/task/json/Configurator.txx"

#endif // smtk_task_json_Helper_h
