//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/json/jsonWorklet.h"
#include "smtk/task/json/Helper.h"

#include "smtk/io/Logger.h"

namespace smtk
{
namespace task
{

void to_json(nlohmann::json& jj, const smtk::task::Worklet::Ptr& worklet)
{
  if (!worklet)
  {
    return;
  }
  jj = worklet->configuration();
}

void from_json(const nlohmann::json& jj, smtk::task::Worklet::Ptr& worklet)
{
  auto& helper = json::Helper::instance();
  auto managers = helper.managers();
  auto& taskManager = helper.taskManager();
  try
  {
    worklet = smtk::task::Worklet::create();
    worklet->configure(jj, taskManager);
  }
  catch (nlohmann::json::exception& e)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Could not deserialize worklet (" << e.what() << ").");
  }
}

} // namespace task
} // namespace smtk
