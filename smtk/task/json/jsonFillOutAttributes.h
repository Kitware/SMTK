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
