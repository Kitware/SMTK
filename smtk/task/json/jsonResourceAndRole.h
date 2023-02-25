//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_json_ResourceAndRole_h
#define smtk_task_json_ResourceAndRole_h

#include "smtk/task/adaptor/ResourceAndRole.h"

#include "nlohmann/json.hpp"

#include <exception>
#include <string>

namespace smtk
{
namespace task
{
namespace json
{

class Helper;

// NB: ResourceAndRole is serialized/deserialized by methods in smtk/task/json/jsonAdaptor.h
//     that use the Configurator to swizzle/unswizzle pointers held by tasks.
//     This file just declares a functor to fetch a ResourceAndRole from the Configurator.

struct SMTKCORE_EXPORT jsonResourceAndRole
{
  Adaptor::Configuration operator()(const Adaptor* task, Helper& helper) const;
};

} // namespace json
} // namespace task
} // namespace smtk

#endif // smtk_task_json_ResourceAndRole_h
