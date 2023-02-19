//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_json_Group_h
#define smtk_task_json_Group_h

#include "smtk/task/Task.h"

#include <exception>
#include <string>

namespace smtk
{
namespace task
{
namespace json
{

class Helper;

// NB: Group is serialized/deserialized by methods in smtk/task/json/jsonTask.h
//     that use the Configurator to swizzle/unswizzle pointers held by tasks.
//     This file just declares a functor to fetch a Group from the Configurator.

struct SMTKCORE_EXPORT jsonGroup
{
  Task::Configuration operator()(const Task* task, Helper& helper) const;
};

} // namespace json
} // namespace task
} // namespace smtk

#endif // smtk_task_json_Group_h
