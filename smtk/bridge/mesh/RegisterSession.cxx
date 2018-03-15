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
#include "smtk/bridge/mesh/RegisterSession.h"

#include "smtk/bridge/mesh/operators/EulerCharacteristicRatio.h"
#include "smtk/bridge/mesh/operators/Export.h"
#include "smtk/bridge/mesh/operators/Import.h"
#include "smtk/bridge/mesh/operators/Read.h"
#include "smtk/bridge/mesh/operators/Write.h"

#include "smtk/bridge/mesh/Resource.h"

#include "smtk/operation/groups/ImporterGroup.h"
#include "smtk/operation/groups/ReaderGroup.h"
#include "smtk/operation/groups/WriterGroup.h"

namespace smtk
{
namespace bridge
{
namespace mesh
{

void registerOperations(smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->registerOperation<smtk::bridge::mesh::EulerCharacteristicRatio>(
    "smtk::bridge::mesh::EulerCharacteristicRatio");
  operationManager->registerOperation<smtk::bridge::mesh::Export>("smtk::bridge::mesh::Export");
  operationManager->registerOperation<smtk::bridge::mesh::Import>("smtk::bridge::mesh::Import");
  operationManager->registerOperation<smtk::bridge::mesh::Read>("smtk::bridge::mesh::Read");
  operationManager->registerOperation<smtk::bridge::mesh::Write>("smtk::bridge::mesh::Write");

  smtk::operation::ImporterGroup(operationManager)
    .registerOperation<smtk::bridge::mesh::Resource, smtk::bridge::mesh::Import>();

  smtk::operation::ReaderGroup(operationManager)
    .registerOperation<smtk::bridge::mesh::Resource, smtk::bridge::mesh::Read>();

  smtk::operation::WriterGroup(operationManager)
    .registerOperation<smtk::bridge::mesh::Resource, smtk::bridge::mesh::Write>();
}

void registerResources(smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->registerResource<smtk::bridge::mesh::Resource>(read, write);
}
}
}
}
