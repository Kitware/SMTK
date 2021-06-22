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
#ifndef smtk_task_Instances_h
#define smtk_task_Instances_h

#include "smtk/common/Instances.h"
#include "smtk/common/Managers.h"
#include "smtk/common/TypeName.h"

#include "smtk/task/Task.h"

namespace smtk
{
namespace task
{

/// Track smtk::task::Task objects with smtk::common::Instances.
using Instances = smtk::common::Instances<
  smtk::task::Task,
  void,
  std::tuple<smtk::task::Task::Configuration&, std::shared_ptr<smtk::common::Managers>>,
  std::tuple<
    smtk::task::Task::Configuration&,
    smtk::task::Task::PassedDependencies,
    std::shared_ptr<smtk::common::Managers>>>;

} // namespace task
} // namespace smtk

#endif // smtk_task_Instances_h
