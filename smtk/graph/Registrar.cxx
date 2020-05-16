//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#include "smtk/graph/Registrar.h"

#include "smtk/graph/Resource.h"

#include "smtk/operation/groups/InternalGroup.h"

#include <tuple>

namespace smtk
{
namespace graph
{
namespace
{
// using OperationList = typedef std::tuple<YourOperationHere,...>;
}

void Registrar::registerTo(const smtk::operation::Manager::Ptr& operationManager)
{
  // operationManager->registerOperations<OperationList>();
}

void Registrar::unregisterFrom(const smtk::operation::Manager::Ptr& operationManager)
{
  // operationManager->unregisterOperations<OperationList>();
}

void Registrar::registerTo(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->registerResource<smtk::graph::Resource>();
}

void Registrar::unregisterFrom(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->unregisterResource<smtk::graph::Resource>();
}

} // namespace graph
} // namespace smtk
