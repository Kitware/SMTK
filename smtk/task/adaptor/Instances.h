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
#ifndef smtk_task_adaptor_Instances_h
#define smtk_task_adaptor_Instances_h

#include "smtk/common/Instances.h"
#include "smtk/common/Managers.h"
#include "smtk/common/TypeName.h"

#include "smtk/task/Task.h"

namespace smtk
{
namespace task
{
namespace adaptor
{

/// Track smtk::task::Adaptor objects with smtk::common::Instances.
using Instances = smtk::common::Instances<
  smtk::task::Adaptor,
  // Constructor variant: default (no arguments)
  void,
  // Constructor variant: configuration, no tasks
  std::tuple<smtk::task::Adaptor::Configuration&>,
  // Constructor variant: configuration and tasks.
  std::tuple<
    smtk::task::Adaptor::Configuration&, // JSON configuration information
    smtk::task::Task*,                   // Source task (from)
    smtk::task::Task*                    // Task to be configured (to)
    >>;

} // namespace adaptor
} // namespace task
} // namespace smtk

#endif // smtk_task_adaptor_Instances_h
