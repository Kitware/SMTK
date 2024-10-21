//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/json/jsonAgent.h"
#include "smtk/task/json/Helper.h"
#include "smtk/task/json/jsonTask.h"

#include "smtk/task/Manager.h"
#include "smtk/task/Port.h"

namespace smtk
{
namespace task
{

void from_json(const nlohmann::json& jj, Agent& agent)
{
  agent.configure(jj);
}

void to_json(nlohmann::json& jj, const Agent& agent)
{
  jj = agent.configuration();
}

} // namespace task
} // namespace smtk
