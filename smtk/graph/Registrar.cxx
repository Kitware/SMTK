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
#include "smtk/graph/operators/CreateArc.h"
#include "smtk/graph/operators/CreateArcType.h"
#include "smtk/graph/operators/DeleteArc.h"

#include "smtk/operation/groups/InternalGroup.h"

#include "smtk/plugin/Manager.h"

#include <tuple>

namespace smtk
{
namespace graph
{
namespace
{
using OperationList = std::tuple<CreateArcType, CreateArc, DeleteArc>;
}

void Registrar::registerTo(const smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->registerOperations<OperationList>();

  // NB: Putting CreateArc in the internal group since graph-resource
  //     subclasses will want to expose a subclassed version.
  smtk::operation::InternalGroup(operationManager).registerOperation<CreateArc>();
}

void Registrar::unregisterFrom(const smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->unregisterOperations<OperationList>();
  smtk::operation::InternalGroup(operationManager).unregisterOperation<CreateArc>();
}

void Registrar::registerTo(const smtk::resource::Manager::Ptr& resourceManager)
{
  // Do not register the concrete graph-resource base class since it has no create method.
  // resourceManager->registerResource<smtk::graph::ResourceBase>();

  auto& typeLabels = resourceManager->objectTypeLabels();
  typeLabels[smtk::common::typeName<smtk::graph::ResourceBase>()] = "graph resource";
  typeLabels[smtk::common::typeName<smtk::graph::Component>()] = "graph node";
}

void Registrar::unregisterFrom(const smtk::resource::Manager::Ptr& resourceManager)
{
  (void)resourceManager;
}

} // namespace graph
} // namespace smtk
