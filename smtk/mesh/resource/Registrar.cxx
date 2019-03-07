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
#include "smtk/mesh/resource/Registrar.h"

#include "smtk/io/ReadMesh.h"
#include "smtk/io/WriteMesh.h"

#include "smtk/mesh/core/Resource.h"

#include "smtk/mesh/operators/DeleteMesh.h"
#include "smtk/mesh/operators/ElevateMesh.h"
#include "smtk/mesh/operators/Export.h"
#include "smtk/mesh/operators/GenerateHotStartData.h"
#include "smtk/mesh/operators/Import.h"
#include "smtk/mesh/operators/InterpolateOntoMesh.h"
#include "smtk/mesh/operators/Read.h"
#include "smtk/mesh/operators/UndoElevateMesh.h"
#include "smtk/mesh/operators/Write.h"

#include "smtk/operation/groups/ExporterGroup.h"
#include "smtk/operation/groups/ImporterGroup.h"
#include "smtk/operation/groups/ReaderGroup.h"
#include "smtk/operation/groups/WriterGroup.h"

namespace smtk
{
namespace mesh
{
namespace
{
typedef std::tuple<DeleteMesh, ElevateMesh, Export, GenerateHotStartData, Import,
  InterpolateOntoMesh, Read, UndoElevateMesh, Write>
  OperationList;
}

void Registrar::registerTo(const smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->registerOperations<OperationList>();

  smtk::operation::ImporterGroup(operationManager)
    .registerOperation<smtk::mesh::Resource, smtk::mesh::Import>();

  smtk::operation::ExporterGroup(operationManager)
    .registerOperation<smtk::mesh::Resource, smtk::mesh::Export>();

  smtk::operation::ReaderGroup(operationManager)
    .registerOperation<smtk::mesh::Resource, smtk::mesh::Read>();

  smtk::operation::WriterGroup(operationManager)
    .registerOperation<smtk::mesh::Resource, smtk::mesh::Write>();
}

void Registrar::unregisterFrom(const smtk::operation::Manager::Ptr& operationManager)
{
  smtk::operation::ImporterGroup(operationManager).unregisterOperation<smtk::mesh::Import>();

  smtk::operation::ExporterGroup(operationManager).unregisterOperation<smtk::mesh::Export>();

  smtk::operation::ReaderGroup(operationManager).unregisterOperation<smtk::mesh::Read>();

  smtk::operation::WriterGroup(operationManager).unregisterOperation<smtk::mesh::Write>();

  operationManager->unregisterOperations<OperationList>();
}

void Registrar::registerTo(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->registerResource<smtk::mesh::Resource>(
    [](const std::string& location) {
      auto resource = smtk::mesh::Resource::create();
      smtk::io::readMesh(location, resource);
      return std::static_pointer_cast<smtk::resource::Resource>(resource);
    },
    [](const smtk::resource::Resource::Ptr& resource) {
      return smtk::io::writeMesh(
        resource->location(), std::static_pointer_cast<smtk::mesh::Resource>(resource));
    });
}

void Registrar::unregisterFrom(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->unregisterResource<smtk::mesh::Resource>();
}
}
}
