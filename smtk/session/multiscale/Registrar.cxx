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
#include "smtk/session/multiscale/Registrar.h"

#include "smtk/session/mesh/operators/Import.h"
#include "smtk/session/mesh/operators/Read.h"
#include "smtk/session/mesh/operators/Write.h"

#include "smtk/session/multiscale/operators/PartitionBoundaries.h"
#include "smtk/session/multiscale/operators/Revolve.h"

#include "smtk/session/multiscale/Resource.h"

#include "smtk/operation/RegisterPythonOperations.h"

#include "smtk/operation/groups/CreatorGroup.h"
#include "smtk/operation/groups/ImporterGroup.h"
#include "smtk/operation/groups/ReaderGroup.h"
#include "smtk/operation/groups/WriterGroup.h"

namespace smtk
{
namespace session
{
namespace multiscale
{

namespace
{
typedef std::tuple<PartitionBoundaries, Revolve> OperationList;
}

void Registrar::registerTo(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->registerResource<smtk::session::multiscale::Resource>(mesh::read, mesh::write);
}

void Registrar::registerTo(const smtk::operation::Manager::Ptr& operationManager)
{
  // Register operations
  operationManager->registerOperations<OperationList>();
  smtk::operation::registerPythonOperations(
    operationManager, "smtk.session.multiscale.import_from_deform");

  smtk::operation::CreatorGroup(operationManager)
    .registerOperation<smtk::session::multiscale::Resource>(
      "smtk.session.multiscale.import_from_deform.import_from_deform");

  smtk::operation::ImporterGroup(operationManager)
    .registerOperation<smtk::session::multiscale::Resource, smtk::session::mesh::Import>();

  smtk::operation::ReaderGroup(operationManager)
    .registerOperation<smtk::session::multiscale::Resource, smtk::session::mesh::Read>();

  smtk::operation::WriterGroup(operationManager)
    .registerOperation<smtk::session::multiscale::Resource, smtk::session::mesh::Write>();
}

void Registrar::unregisterFrom(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->unregisterResource<smtk::session::mesh::Resource>();
}

void Registrar::unregisterFrom(const smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->unregisterOperations<OperationList>();
  operationManager->unregisterOperation(
    "smtk.session.multiscale.import_from_deform.import_from_deform");
}
}
}
}
