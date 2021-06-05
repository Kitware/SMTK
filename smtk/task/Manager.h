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

#include "smtk/task/Factory.h"

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

  /// Return a factory object used to register task types and create tasks.
  Factory& taskFactory() { return m_taskFactory; }
  const Factory& taskFactory() const { return m_taskFactory; }

  /// Return the managers instance that contains this manager, if it exists.
  smtk::common::Managers::Ptr managers() const { return m_managers.lock(); }
  void setManagers(const smtk::common::Managers::Ptr& managers) { m_managers = managers; }

private:
  Factory m_taskFactory;
  std::weak_ptr<smtk::common::Managers> m_managers;

protected:
  Manager();
};
} // namespace task
} // namespace smtk

#endif // smtk_task_Manager_h
