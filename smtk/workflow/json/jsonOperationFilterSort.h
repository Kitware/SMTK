//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_workflow_json_jsonOperationFilterSort_h
#define smtk_workflow_json_jsonOperationFilterSort_h

#include "smtk/CoreExports.h"

#include "smtk/workflow/OperationFilterSort.h"

#include "nlohmann/json.hpp"

#include <exception>
#include <string>

namespace smtk
{
namespace workflow
{

using json = nlohmann::json;

SMTKCORE_EXPORT void to_json(
  json& j, const OperationFilterSortPtr& ofs, smtk::operation::ManagerPtr opMgr);
SMTKCORE_EXPORT void from_json(
  const json& j, OperationFilterSortPtr& ofs, smtk::operation::ManagerPtr opMgr);
}
}

#endif
