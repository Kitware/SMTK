//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_WorkflowObjectMonitor_h
#define smtk_task_WorkflowObjectMonitor_h

#include "smtk/task/Manager.h"

namespace smtk
{
namespace task
{

/**\brief Monitor a set of workflow objects for changes.
  *
  * Changes may be due to (1) membership changes or (2) modifications
  * made to member objects by operations.
  *
  * Consumers of this class should own an instance of it
  * (to keep its operation observer alive) and provide callbacks
  * to be invoked when changes are made.
  */
class WorkflowObjectMonitor
{
public:
  smtkTypeMacroBase(smtk::task::WorkflowObjectMonitor);

  using ObjectAddedNotifier =
    std::function<void(const std::shared_ptr<smtk::resource::PersistentObject>&)>;
  using ObjectRemovedNotifier =
    std::function<void(const std::shared_ptr<smtk::resource::PersistentObject>&)>;
  using ObjectModifiedNotifier =
    std::function<void(const std::shared_ptr<smtk::resource::PersistentObject>&)>;

  WorkflowObjectMonitor(
    smtk::task::Task* task,
    const nlohmann::json& spec,
    ObjectAddedNotifier addNotifier,
    ObjectRemovedNotifier removeNotifier,
    ObjectModifiedNotifier modifiedNotifier);
  WorkflowObjectMonitor(
    smtk::task::Task* task,
    const smtk::view::Configuration::Component& spec,
    ObjectAddedNotifier addNotifier,
    ObjectRemovedNotifier removeNotifier,
    ObjectModifiedNotifier modifiedNotifier);
  ~WorkflowObjectMonitor();

protected:
  smtk::task::Task* m_task{ nullptr };
  nlohmann::json m_spec;
  ObjectAddedNotifier m_addNotifier;
  ObjectRemovedNotifier m_removeNotifier;
  ObjectModifiedNotifier m_modifiedNotifier;
  class Internal;
  std::unique_ptr<Internal> m_p;
};

} // namespace task
} // namespace smtk

#endif // smtk_task_WorkflowObjectMonitor_h
