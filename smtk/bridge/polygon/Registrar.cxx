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
#include "smtk/bridge/polygon/Registrar.h"

#include "smtk/bridge/polygon/operators/CleanGeometry.h"
#include "smtk/bridge/polygon/operators/CreateEdge.h"
#include "smtk/bridge/polygon/operators/CreateEdgeFromPoints.h"
#include "smtk/bridge/polygon/operators/CreateEdgeFromVertices.h"
#include "smtk/bridge/polygon/operators/CreateFaces.h"
#include "smtk/bridge/polygon/operators/CreateFacesFromEdges.h"
#include "smtk/bridge/polygon/operators/CreateModel.h"
#include "smtk/bridge/polygon/operators/CreateVertices.h"
#include "smtk/bridge/polygon/operators/Delete.h"
#include "smtk/bridge/polygon/operators/DemoteVertex.h"
#include "smtk/bridge/polygon/operators/ExtractContours.h"
#include "smtk/bridge/polygon/operators/ForceCreateFace.h"
#include "smtk/bridge/polygon/operators/Import.h"
#include "smtk/bridge/polygon/operators/LegacyRead.h"
#include "smtk/bridge/polygon/operators/Read.h"
#include "smtk/bridge/polygon/operators/SplitEdge.h"
#include "smtk/bridge/polygon/operators/TweakEdge.h"
#include "smtk/bridge/polygon/operators/Write.h"

#include "smtk/bridge/polygon/Resource.h"

#include "smtk/operation/groups/ImporterGroup.h"
#include "smtk/operation/groups/ReaderGroup.h"
#include "smtk/operation/groups/WriterGroup.h"

namespace smtk
{
namespace bridge
{
namespace polygon
{

namespace
{
typedef std::tuple<CleanGeometry, CreateEdge, CreateEdgeFromPoints, CreateEdgeFromVertices,
  CreateFaces, CreateFacesFromEdges, CreateModel, CreateVertices, Delete, DemoteVertex,
  ExtractContours, ForceCreateFace, Import, LegacyRead, Read, SplitEdge, TweakEdge, Write>
  OperationList;
}

void Registrar::registerTo(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->registerResource<smtk::bridge::polygon::Resource>(read, write);
}

void Registrar::registerTo(const smtk::operation::Manager::Ptr& operationManager)
{
  // Register operations
  operationManager->registerOperations<OperationList>();

  smtk::operation::ImporterGroup(operationManager)
    .registerOperation<smtk::bridge::polygon::Resource, smtk::bridge::polygon::Import>();

  smtk::operation::ReaderGroup(operationManager)
    .registerOperation<smtk::bridge::polygon::Resource, smtk::bridge::polygon::Read>();
  smtk::operation::ReaderGroup(operationManager)
    .registerOperation(smtk::common::typeName<smtk::bridge::polygon::LegacyRead>(), "polygon");

  smtk::operation::WriterGroup(operationManager)
    .registerOperation<smtk::bridge::polygon::Resource, smtk::bridge::polygon::Write>();
}

void Registrar::unregisterFrom(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->unregisterResource<smtk::bridge::polygon::Resource>();
}

void Registrar::unregisterFrom(const smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->unregisterOperations<OperationList>();
}
}
}
}
