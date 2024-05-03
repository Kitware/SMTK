//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
/*! \file */
#ifndef smtk_task_PortInstances_h
#define smtk_task_PortInstances_h

#include "smtk/common/Instances.h"
#include "smtk/common/Managers.h"
#include "smtk/common/TypeName.h"

#include "smtk/string/Token.h"

#include "smtk/task/Port.h"

namespace smtk
{
namespace task
{
class Manager;

using PortInstancesBase = smtk::common::Instances<
  smtk::task::Port,
  void,
  std::tuple<
    const smtk::task::Port::Configuration&,
    smtk::task::Manager&,
    std::shared_ptr<smtk::common::Managers>>,
  std::tuple<
    const smtk::task::Port::Configuration&,
    smtk::task::Task*,
    std::shared_ptr<smtk::common::Managers>>>;

/// Track smtk::task::Port objects with smtk::common::Instances.
///
/// This class adds methods to create tasks by name and will
/// eventually index tasks by UUID and name.
class SMTKCORE_EXPORT PortInstances : public PortInstancesBase
{
public:
  smtkTypeMacroBase(smtk::task::PortInstances);
  smtkSuperclassMacro(smtk::task::PortInstancesBase);

  PortInstances(Manager& taskManager);
  PortInstances(const PortInstances&) = delete;
  void operator=(const PortInstances&) = delete;
  virtual ~PortInstances() = default;

  ///@{
  /// Create a task given its class name and optionally more configuration data.
  ///
  /// These override the base factory methods by supplying the task manager.
  Port::Ptr createFromName(const std::string& taskType);
  Port::Ptr createFromName(
    const std::string& taskType,
    const Port::Configuration& configuration,
    std::shared_ptr<smtk::common::Managers> managers);
  Port::Ptr createFromName(
    const std::string& taskType,
    const Port::Configuration& configuration,
    smtk::task::Task* parentTask,
    std::shared_ptr<smtk::common::Managers> managers);
  ///@}

  /// Returns the tasks with the given name
  std::set<smtk::task::Port::Ptr> findByName(const std::string& name) const;

  /// Returns the task with the given ID.
  smtk::task::Port::Ptr findById(const smtk::common::UUID& taskId) const;

protected:
  Manager& m_taskManager;
};

} // namespace task
} // namespace smtk

#endif // smtk_task_PortInstances_h
