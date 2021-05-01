//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/project/json/jsonOperationFactory.h"

// Define how projects are serialized.
namespace smtk
{
namespace project
{
void to_json(json& j, const OperationFactory& operationFactory)
{
  j["types"] = operationFactory.types();
}

void from_json(const json& j, OperationFactory& operationFactory)
{
  operationFactory.types() = j["types"].get<std::set<std::string>>();
}
} // namespace project
} // namespace smtk
