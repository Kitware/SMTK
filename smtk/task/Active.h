//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_task_Active_h
#define smtk_task_Active_h

#include "smtk/CoreExports.h"
#include "smtk/task/Task.h"

namespace smtk
{
namespace task
{

class Instances;
class Task;

/// This object provides applications a way to change and observe the active task.
///
/// If passed an smtk::task::Manager::Instances object at construction,
/// only managed tasks may be active.
/// This ensures that before a task is destroyed this object
/// can notify observers the active task is becoming inactive.
///
/// If not passed an smtk::task::Manager::Instances object at construction,
/// any task may become active but
/// unmanaged tasks might be deleted without any notification that the active
/// task changed.
/// You are strongly encouraged to pass Instances to the constructor.
class SMTKCORE_EXPORT Active
{
public:
  smtkTypeMacroBase(smtk::task::Active);

  /// The signature for observers of the active task.
  /// Observers are passed the previously-active task and the soon-to-be-active task.
  ///
  /// Note that either task may be null (i.e., it is possible to have no active task).
  using Observer = std::function<void(smtk::task::Task*, smtk::task::Task*)>;
  /// The container for registered observers.
  using Observers = smtk::common::Observers<smtk::task::Active::Observer>;

  /// Construct an active-task tracker.
  Active(smtk::task::Instances* instances = nullptr);
  virtual ~Active();

  /// Return the active task (or nullptr if no task is active).
  smtk::task::Task* task() const;

  /// Change the active task (or abandon the currently-active task by passing nullptr).
  ///
  /// This method returns true if the active task changed and false otherwise.
  /// Passing a task that is not managed by \a instances passed to the constructor
  /// will always return false.
  /// Passing a task that is already active or unavailable will return false.
  bool switchTo(smtk::task::Task*);

  /// Return the set of active-task observers (so you can insert yourself).
  Observers& observers();
  const Observers& observers() const;

private:
  // We declare a subclass to hold some internal state to avoid a
  // cyclic header dependency (Instances requires Managers requires Active).
  struct Internal;
  Internal* m_p;
};
} // namespace task
} // namespace smtk

#endif // smtk_task_Active_h
