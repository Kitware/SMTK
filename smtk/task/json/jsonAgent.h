//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_json_Agent_h
#define smtk_task_json_Agent_h

#include "smtk/task/Agent.h"

#include "nlohmann/json.hpp"

#include <exception>
#include <string>

namespace smtk
{
namespace task
{

void SMTKCORE_EXPORT from_json(const nlohmann::json& j, Agent& agent);
void SMTKCORE_EXPORT to_json(nlohmann::json& j, const Agent& agent);

} // namespace task
} // namespace smtk

#endif // smtk_task_json_Agent_h
