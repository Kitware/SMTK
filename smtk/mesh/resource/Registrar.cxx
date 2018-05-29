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

#include "smtk/mesh/core/Collection.h"

#include "smtk/mesh/operators/DeleteMesh.h"
#include "smtk/mesh/operators/ElevateMesh.h"
#include "smtk/mesh/operators/ExportMesh.h"
#include "smtk/mesh/operators/GenerateHotStartData.h"
#include "smtk/mesh/operators/InterpolateOntoMesh.h"
#include "smtk/mesh/operators/UndoElevateMesh.h"
#include "smtk/mesh/operators/WriteMesh.h"

namespace smtk
{
namespace mesh
{
namespace
{
typedef std::tuple<DeleteMesh, ElevateMesh, ExportMesh, GenerateHotStartData, InterpolateOntoMesh,
  UndoElevateMesh, WriteMesh>
  OperationList;
}

void Registrar::registerTo(const smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->registerOperations<OperationList>();
}

void Registrar::unregisterFrom(const smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->unregisterOperations<OperationList>();
}

void Registrar::registerTo(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->registerResource<smtk::mesh::Collection>();
}

void Registrar::unregisterFrom(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->unregisterResource<smtk::mesh::Collection>();
}
}
}
