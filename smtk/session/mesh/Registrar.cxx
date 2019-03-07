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
#include "smtk/session/mesh/Registrar.h"

#include "smtk/session/mesh/operators/CreateUniformGrid.h"
#include "smtk/session/mesh/operators/EulerCharacteristicRatio.h"
#include "smtk/session/mesh/operators/Export.h"
#include "smtk/session/mesh/operators/Import.h"
#include "smtk/session/mesh/operators/Read.h"
#include "smtk/session/mesh/operators/Write.h"

#include "smtk/session/mesh/Resource.h"

#include "smtk/operation/groups/CreatorGroup.h"
#include "smtk/operation/groups/ImporterGroup.h"
#include "smtk/operation/groups/ReaderGroup.h"
#include "smtk/operation/groups/WriterGroup.h"

namespace smtk
{
namespace session
{
namespace mesh
{

namespace
{
typedef std::tuple<CreateUniformGrid, smtk::session::mesh::EulerCharacteristicRatio, Export, Import,
  Read, Write>
  OperationList;
}

void Registrar::registerTo(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->registerResource<smtk::session::mesh::Resource>(read, write);
}

void Registrar::registerTo(const smtk::operation::Manager::Ptr& operationManager)
{
  // Register operations
  operationManager->registerOperations<OperationList>();

  smtk::operation::CreatorGroup(operationManager)
    .registerOperation<smtk::session::mesh::Resource, smtk::session::mesh::CreateUniformGrid>();

  smtk::operation::ImporterGroup(operationManager)
    .registerOperation<smtk::session::mesh::Resource, smtk::session::mesh::Import>();

  smtk::operation::ReaderGroup(operationManager)
    .registerOperation<smtk::session::mesh::Resource, smtk::session::mesh::Read>();

  smtk::operation::WriterGroup(operationManager)
    .registerOperation<smtk::session::mesh::Resource, smtk::session::mesh::Write>();
}

void Registrar::unregisterFrom(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->unregisterResource<smtk::session::mesh::Resource>();
}

void Registrar::unregisterFrom(const smtk::operation::Manager::Ptr& operationManager)
{
  smtk::operation::CreatorGroup(operationManager)
    .unregisterOperation<smtk::session::mesh::CreateUniformGrid>();

  smtk::operation::ImporterGroup(operationManager)
    .unregisterOperation<smtk::session::mesh::Import>();

  smtk::operation::ReaderGroup(operationManager).unregisterOperation<smtk::session::mesh::Read>();

  smtk::operation::WriterGroup(operationManager).unregisterOperation<smtk::session::mesh::Write>();

  operationManager->unregisterOperations<OperationList>();
}
}
}
}
