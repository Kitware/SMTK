//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/opencascade/Registrar.h"

#include "smtk/session/opencascade/IconConstructor.h"
#include "smtk/session/opencascade/Resource.h"
#include "smtk/session/opencascade/operators/Import.h"

#include "smtk/operation/groups/ImporterGroup.h"

namespace smtk
{
namespace session
{
namespace opencascade
{

namespace
{
using OperationList = std::tuple<Import>;
}

void Registrar::registerTo(const smtk::resource::Manager::Ptr& resourceManager)
{
  // resource type to a manager
  resourceManager->registerResource<smtk::session::opencascade::Resource>();
}

void Registrar::registerTo(const smtk::operation::Manager::Ptr& operationManager)
{
  // Register operations to the operation manager
  operationManager->registerOperations<OperationList>();

  smtk::operation::ImporterGroup(operationManager)
    .registerOperation<smtk::session::opencascade::Resource, smtk::session::opencascade::Import>();
}

void Registrar::registerTo(const smtk::view::Manager::Ptr& viewManager)
{
  viewManager->iconFactory().registerIconConstructor<Resource>(IconConstructor());
}

void Registrar::unregisterFrom(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->unregisterResource<smtk::session::opencascade::Resource>();
}

void Registrar::unregisterFrom(const smtk::operation::Manager::Ptr& operationManager)
{
  smtk::operation::ImporterGroup(operationManager)
    .unregisterOperation<smtk::session::opencascade::Import>();

  operationManager->unregisterOperations<OperationList>();
}

void Registrar::unregisterFrom(const smtk::view::Manager::Ptr& viewManager)
{
  (void)viewManager;
  // TODO: Unregister icons?
}
}
}
}
