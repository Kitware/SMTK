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

#include <tuple>

namespace smtk
{
namespace mesh
{

typedef std::tuple<DeleteMesh, ElevateMesh, ExportMesh, GenerateHotStartData, InterpolateOntoMesh,
  UndoElevateMesh, WriteMesh>
  OperationList;

void registerOperations(smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->registerOperations<OperationList>();
}

void unregisterOperations(smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->unregisterOperations<OperationList>();
}
}
}
