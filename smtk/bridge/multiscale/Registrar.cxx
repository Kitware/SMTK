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
#include "smtk/bridge/multiscale/Registrar.h"

#include "smtk/bridge/mesh/operators/Import.h"
#include "smtk/bridge/mesh/operators/Read.h"
#include "smtk/bridge/mesh/operators/Write.h"

#include "smtk/bridge/multiscale/operators/PartitionBoundaries.h"
#include "smtk/bridge/multiscale/operators/Revolve.h"

#include "smtk/bridge/multiscale/Resource.h"

#include "smtk/operation/RegisterPythonOperations.h"

#include "smtk/operation/groups/CreatorGroup.h"
#include "smtk/operation/groups/ImporterGroup.h"
#include "smtk/operation/groups/ReaderGroup.h"
#include "smtk/operation/groups/WriterGroup.h"

namespace smtk
{
namespace bridge
{
namespace multiscale
{

namespace
{
typedef std::tuple<PartitionBoundaries, Revolve> OperationList;
}

void Registrar::registerTo(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->registerResource<smtk::bridge::multiscale::Resource>(mesh::read, mesh::write);
}

void Registrar::registerTo(const smtk::operation::Manager::Ptr& operationManager)
{
  // Register operations
  operationManager->registerOperations<OperationList>();
  smtk::operation::registerPythonOperations(
    operationManager, "smtk.bridge.multiscale.import_from_deform");

  smtk::operation::CreatorGroup(operationManager)
    .registerOperation<smtk::bridge::multiscale::Resource>(
      "smtk.bridge.multiscale.import_from_deform.import_from_deform");

  smtk::operation::ImporterGroup(operationManager)
    .registerOperation<smtk::bridge::multiscale::Resource, smtk::bridge::mesh::Import>();

  smtk::operation::ReaderGroup(operationManager)
    .registerOperation<smtk::bridge::multiscale::Resource, smtk::bridge::mesh::Read>();

  smtk::operation::WriterGroup(operationManager)
    .registerOperation<smtk::bridge::multiscale::Resource, smtk::bridge::mesh::Write>();
}

void Registrar::unregisterFrom(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->unregisterResource<smtk::bridge::mesh::Resource>();
}

void Registrar::unregisterFrom(const smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->unregisterOperations<OperationList>();
  operationManager->unregisterOperation(
    "smtk.bridge.multiscale.import_from_deform.import_from_deform");
}
}
}
}
