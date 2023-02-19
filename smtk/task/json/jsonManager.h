//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_json_Manager_h
#define smtk_task_json_Manager_h

#include "smtk/task/Manager.h"

#include "smtk/common/Managers.h"

#include "nlohmann/json.hpp"

namespace smtk
{
namespace task
{

///@{
/// Serialize/deserialize a task manager.
///
/// Note that the caller **must** push a smtk::task::json::Helper onto
/// the stack before calling these methods and the helper must have its
/// smtk::common::Managers set in order for these methods to work.
void SMTKCORE_EXPORT from_json(const nlohmann::json& j, Manager& taskManager);
void SMTKCORE_EXPORT to_json(nlohmann::json& j, const Manager& taskManager);
///@}

} // namespace task
} // namespace smtk

#endif // smtk_task_json_Manager_h
