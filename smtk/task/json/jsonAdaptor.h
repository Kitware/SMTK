//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_task_json_Adaptor_h
#define smtk_task_json_Adaptor_h

#include "nlohmann/json.hpp"

#include "smtk/task/Adaptor.h"

#include <exception>
#include <string>

namespace smtk
{
namespace task
{
namespace json
{

class Helper;

struct SMTKCORE_EXPORT jsonAdaptor
{
  Adaptor::Configuration operator()(const Adaptor* adaptor, Helper& helper) const;
};

} // namespace json

SMTKCORE_EXPORT void to_json(nlohmann::json& j, const smtk::task::Adaptor::Ptr& adaptor);

SMTKCORE_EXPORT void from_json(const nlohmann::json& j, smtk::task::Adaptor::Ptr& adaptor);

} // namespace task
} // namespace smtk

#endif // smtk_task_json_Adaptor_h
