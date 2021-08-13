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
/// This is needed in order to serialized dependencies among tasks which
/// are stored as pointers that could, in theory, form a cycle.
class SMTKCORE_EXPORT Helper
{
public:
  /// JSON data type
  using json = nlohmann::json;

  /// Destructor is public, but you shouldn't use it.
  ~Helper();

  /// Return the helper singleton.
  static Helper& instance();

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

  void setAdaptorTaskIds(std::size_t fromId, std::size_t toId);
  void clearAdaptorTaskIds();
  std::pair<Task::Ptr, Task::Ptr> getAdaptorTasks();

protected:
  Helper();
  Configurator<Task> m_tasks;
  Configurator<Adaptor> m_adaptors;
  smtk::common::Managers::Ptr m_managers;
  std::size_t m_adaptorFromId = ~0;
  std::size_t m_adaptorToId = ~0;
};

} // namespace json
} // namespace task
} // namespace smtk

// Include the configurator implementation here since
// it requires the Helper class defined.
#include "smtk/task/json/Configurator.txx"

#endif // smtk_task_json_Helper_h
