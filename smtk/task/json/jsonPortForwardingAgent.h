//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_json_PortForwardingAgent_h
#define smtk_task_json_PortForwardingAgent_h

#include "smtk/task/PortForwardingAgent.h"
#include "smtk/task/json/jsonAgent.h"

#include "nlohmann/json.hpp"

#include <exception>
#include <string>

namespace smtk
{
namespace task
{

void SMTKCORE_EXPORT
from_json(const nlohmann::json& j, PortForwardingAgent::ObjectFilter& objectFilter);
void SMTKCORE_EXPORT
to_json(nlohmann::json& j, const PortForwardingAgent::ObjectFilter& objectFilter);

} // namespace task
} // namespace smtk

#endif // smtk_task_json_PortForwardingAgent_h
