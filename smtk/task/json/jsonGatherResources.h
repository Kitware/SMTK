//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_json_GatherResources_h
#define smtk_task_json_GatherResources_h

#include "smtk/task/GatherResources.h"

#include <exception>
#include <string>

namespace smtk
{
namespace task
{

// NB: GatherResources is serialized/deserialized by methods in smtk/task/json/jsonTask.h
//     that use the Configurator to swizzle/unswizzle pointers held by tasks.
//     This file just adds to_json/from_json methods for member variables of GatherResources
//     and declares a functor to fetch a GatherResources from the Configurator.

void from_json(const nlohmann::json& j, GatherResources::ResourceSet& resourceSet);
void to_json(nlohmann::json& j, const GatherResources::ResourceSet& resourceSet);

namespace json
{

class Helper;

struct SMTKCORE_EXPORT jsonGatherResources
{
  Task::Configuration operator()(const Task* task, Helper& helper) const;
};

} // namespace json
} // namespace task
} // namespace smtk

#endif // smtk_task_json_GatherResources_h
