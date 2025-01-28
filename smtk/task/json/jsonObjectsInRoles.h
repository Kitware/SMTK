//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_json_ObjectsInRoles_h
#define smtk_task_json_ObjectsInRoles_h

#include "smtk/task/ObjectsInRoles.h"

#include "nlohmann/json.hpp"

#include <exception>
#include <string>

namespace smtk
{
namespace task
{

// NB: ObjectsInRoles is serialized/deserialized by methods in smtk/task/json/Helper.h
//     that use the Configurator to swizzle/unswizzle pointers held by tasks, ports, etc..

// clang-format off
void SMTKCORE_EXPORT from_json(const nlohmann::json& jj, std::shared_ptr<ObjectsInRoles>& objectsInRoles);
void SMTKCORE_EXPORT to_json(nlohmann::json& jj, const std::shared_ptr<ObjectsInRoles>& objectsInRoles);
// clang-format on

} // namespace task
} // namespace smtk

#endif // smtk_task_json_ObjectsInRoles_h
