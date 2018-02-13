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
#include "smtk/mesh/operators/RegisterOperations.h"

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

void registerOperations(smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->registerOperation<smtk::mesh::DeleteMesh>("smtk::mesh::DeleteMesh");
  operationManager->registerOperation<smtk::mesh::ElevateMesh>("smtk::mesh::ElevateMesh");
  operationManager->registerOperation<smtk::mesh::ExportMesh>("smtk::mesh::ExportMesh");
  operationManager->registerOperation<smtk::mesh::GenerateHotStartData>(
    "smtk::mesh::GenerateHotStartData");
  operationManager->registerOperation<smtk::mesh::InterpolateOntoMesh>(
    "smtk::mesh::InterpolateOntoMesh");
  operationManager->registerOperation<smtk::mesh::UndoElevateMesh>("smtk::mesh::UndoElevateMesh");
  operationManager->registerOperation<smtk::mesh::WriteMesh>("smtk::mesh::WriteMesh");
}
}
}
