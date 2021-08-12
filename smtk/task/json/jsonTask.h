//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_json_Task_h
#define smtk_task_json_Task_h

#include "nlohmann/json.hpp"

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

struct SMTKCORE_EXPORT jsonTask
{
  Task::Configuration operator()(const Task* task, Helper& helper) const;
};

} // namespace json

SMTKCORE_EXPORT void to_json(nlohmann::json& j, const smtk::task::Task::Ptr& task);

SMTKCORE_EXPORT void from_json(const nlohmann::json& j, smtk::task::Task::Ptr& task);

} // namespace task
} // namespace smtk

#endif // smtk_task_json_Task_h
