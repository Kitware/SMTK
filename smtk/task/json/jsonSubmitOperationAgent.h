//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_json_SubmitOperationAgent_h
#define smtk_task_json_SubmitOperationAgent_h

#include "smtk/task/SubmitOperationAgent.h"

#include "nlohmann/json.hpp"

#include <exception>
#include <string>

namespace smtk
{
namespace task
{

// NB: SubmitOperationAgent is serialized/deserialized by methods in smtk/task/json/jsonTask.h
//     that use the Configurator to swizzle/unswizzle pointers held by tasks.
//     This file just adds to_json/from_json methods for member variables of SubmitOperationAgent
//     and declares a functor to fetch a SubmitOperationAgent from the Configurator.

// clang-format off
void SMTKCORE_EXPORT from_json(const nlohmann::json& jj, SubmitOperationAgent::ParameterSpec& parameterSpec);
void SMTKCORE_EXPORT to_json(nlohmann::json& jj, const SubmitOperationAgent::ParameterSpec& parameterSpec);

void SMTKCORE_EXPORT from_json(const nlohmann::json& jj, SubmitOperationAgent::ParameterSpecRef& specRef);
void SMTKCORE_EXPORT to_json(nlohmann::json& jj, const SubmitOperationAgent::ParameterSpecRef& specRef);
// clang-format on

} // namespace task
} // namespace smtk

#endif // smtk_task_json_SubmitOperationAgent_h
