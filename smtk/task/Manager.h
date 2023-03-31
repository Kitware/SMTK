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
#include "smtk/string/Token.h"

#include "smtk/task/Active.h"
#include "smtk/task/Adaptor.h"
#include "smtk/task/Instances.h"
#include "smtk/task/Task.h"
#include "smtk/task/UIState.h"
#include "smtk/task/adaptor/Instances.h"

#include "nlohmann/json.hpp"

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

  Manager();
  virtual ~Manager();
  Manager(const Manager&) = delete;
  void operator=(const Manager&) = delete;

  /// Managed instances of Task objects (and a registry of Task classes).
  using TaskInstances = smtk::task::Instances;

  /// Return the set of managed task instances.
  ///
  /// This class also acts as a registrar for Task subclasses.
  TaskInstances& taskInstances() { return m_taskInstances; }
  const TaskInstances& taskInstances() const { return m_taskInstances; }

  /// Return a tracker for the active task.
  Active& active() { return m_active; }
  const Active& active() const { return m_active; }

  /// Managed instances of Adaptor objects (and a registry of Adaptor classes).
  using AdaptorInstances = smtk::task::adaptor::Instances;

  /// Return the set of managed adaptor instances.
  ///
  /// This class also acts as a registrar for Adaptor subclasses.
  AdaptorInstances& adaptorInstances() { return m_adaptorInstances; }
  const AdaptorInstances& adaptorInstances() const { return m_adaptorInstances; }

  /// Return the managers instance that contains this manager, if it exists.
  smtk::common::Managers::Ptr managers() const { return m_managers.lock(); }
  void setManagers(const smtk::common::Managers::Ptr& managers) { m_managers = managers; }

  /// Given a style key, return a style config.
  nlohmann::json getStyle(const smtk::string::Token& styleClass) const;
  nlohmann::json getStyles() const { return m_styles; };
  void setStyles(const nlohmann::json& styles) { m_styles = styles; }

  /// Store geometry changes from UI components
  UIState& uiState() { return m_uiState; }

private:
  TaskInstances m_taskInstances;
  AdaptorInstances m_adaptorInstances;
  Active m_active;
  std::weak_ptr<smtk::common::Managers> m_managers;
  nlohmann::json m_styles;
  UIState m_uiState;
};
} // namespace task
} // namespace smtk

#endif // smtk_task_Manager_h
