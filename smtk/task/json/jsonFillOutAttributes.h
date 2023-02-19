//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_json_FillOutAttributes_h
#define smtk_task_json_FillOutAttributes_h

#include "smtk/task/FillOutAttributes.h"

#include "nlohmann/json.hpp"

#include <exception>
#include <string>

namespace smtk
{
namespace task
{

// NB: FillOutAttributes is serialized/deserialized by methods in smtk/task/json/jsonTask.h
//     that use the Configurator to swizzle/unswizzle pointers held by tasks.
//     This file just adds to_json/from_json methods for member variables of FillOutAttributes
//     and declares a functor to fetch a FillOutAttributes from the Configurator.

void SMTKCORE_EXPORT
from_json(const nlohmann::json& j, FillOutAttributes::AttributeSet& attributeSet);
void SMTKCORE_EXPORT
to_json(nlohmann::json& j, const FillOutAttributes::AttributeSet& attributeSet);
void SMTKCORE_EXPORT
from_json(const nlohmann::json& j, FillOutAttributes::ResourceAttributes& resourceAttributes);
void SMTKCORE_EXPORT
to_json(nlohmann::json& j, const FillOutAttributes::ResourceAttributes& resourceAttributes);

namespace json
{

class Helper;

struct SMTKCORE_EXPORT jsonFillOutAttributes
{
  Task::Configuration operator()(const Task* task, Helper& helper) const;
};

} // namespace json
} // namespace task
} // namespace smtk

#endif // smtk_task_json_FillOutAttributes_h
