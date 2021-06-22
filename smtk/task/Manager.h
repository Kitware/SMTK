//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_task_Manager_h
#define smtk_task_Manager_h

#include "smtk/CoreExports.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/SharedFromThis.h"

#include "smtk/common/Managers.h"
#include "smtk/common/TypeName.h"

#include "smtk/task/Active.h"
#include "smtk/task/Instances.h"
#include "smtk/task/Task.h"

#include <array>
#include <string>
#include <tuple>
#include <type_traits>
#include <typeinfo>

namespace smtk
{
namespace task
{

/// A task manager is responsible for creating new tasks.
///
/// Eventually, the task manager will also hold an inventory
/// of created tasks and be a clearinghouse for task state transitions.
class SMTKCORE_EXPORT Manager : smtkEnableSharedPtr(Manager)
{
public:
  smtkTypeMacroBase(smtk::task::Manager);
  smtkCreateMacro(smtk::task::Manager);

  virtual ~Manager();

  /// Managed instances of Task objects (and a registry of Task classes).
  using Instances = smtk::task::Instances;

  /// Return the set of managed task instances.
  ///
  /// This class also acts as a registrar for Task subclasses.
  Instances& instances() { return m_instances; }
  const Instances& instances() const { return m_instances; }

  /// Return a tracker for the active task.
  Active& active() { return m_active; }
  const Active& active() const { return m_active; }

  /// Return the managers instance that contains this manager, if it exists.
  smtk::common::Managers::Ptr managers() const { return m_managers.lock(); }
  void setManagers(const smtk::common::Managers::Ptr& managers) { m_managers = managers; }

private:
  Instances m_instances;
  Active m_active;
  std::weak_ptr<smtk::common::Managers> m_managers;

protected:
  Manager();
};
} // namespace task
} // namespace smtk

#endif // smtk_task_Manager_h
