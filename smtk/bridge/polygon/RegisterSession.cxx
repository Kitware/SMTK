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
#include "smtk/bridge/polygon/RegisterSession.h"

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

void registerOperations(smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->registerOperation<smtk::bridge::polygon::CleanGeometry>(
    "smtk::bridge::polygon::CleanGeometry");
  operationManager->registerOperation<smtk::bridge::polygon::CreateEdge>(
    "smtk::bridge::polygon::CreateEdge");
  operationManager->registerOperation<smtk::bridge::polygon::CreateEdgeFromPoints>(
    "smtk::bridge::polygon::CreateEdgeFromPoints");
  operationManager->registerOperation<smtk::bridge::polygon::CreateEdgeFromVertices>(
    "smtk::bridge::polygon::CreateEdgeFromVertices");
  operationManager->registerOperation<smtk::bridge::polygon::CreateFaces>(
    "smtk::bridge::polygon::CreateFaces");
  operationManager->registerOperation<smtk::bridge::polygon::CreateFacesFromEdges>(
    "smtk::bridge::polygon::CreateFacesFromEdges");
  operationManager->registerOperation<smtk::bridge::polygon::CreateModel>(
    "smtk::bridge::polygon::CreateModel");
  operationManager->registerOperation<smtk::bridge::polygon::CreateVertices>(
    "smtk::bridge::polygon::CreateVertices");
  operationManager->registerOperation<smtk::bridge::polygon::Delete>(
    "smtk::bridge::polygon::Delete");
  operationManager->registerOperation<smtk::bridge::polygon::DemoteVertex>(
    "smtk::bridge::polygon::DemoteVertex");
  operationManager->registerOperation<smtk::bridge::polygon::ExtractContours>(
    "smtk::bridge::polygon::ExtractContours");
  operationManager->registerOperation<smtk::bridge::polygon::ForceCreateFace>(
    "smtk::bridge::polygon::ForceCreateFace");
  operationManager->registerOperation<smtk::bridge::polygon::Import>(
    "smtk::bridge::polygon::Import");
  operationManager->registerOperation<smtk::bridge::polygon::LegacyRead>(
    "smtk::bridge::polygon::LegacyRead");
  operationManager->registerOperation<smtk::bridge::polygon::Read>("smtk::bridge::polygon::Read");
  operationManager->registerOperation<smtk::bridge::polygon::SplitEdge>(
    "smtk::bridge::polygon::SplitEdge");
  operationManager->registerOperation<smtk::bridge::polygon::TweakEdge>(
    "smtk::bridge::polygon::TweakEdge");
  operationManager->registerOperation<smtk::bridge::polygon::Write>("smtk::bridge::polygon::Write");

  smtk::operation::ImporterGroup(operationManager)
    .registerOperation<smtk::bridge::polygon::Resource, smtk::bridge::polygon::Import>();

  smtk::operation::ReaderGroup(operationManager)
    .registerOperation<smtk::bridge::polygon::Resource, smtk::bridge::polygon::Read>();
  smtk::operation::ReaderGroup(operationManager)
    .registerOperation("smtk::bridge::polygon::LegacyRead", "polygon");

  smtk::operation::WriterGroup(operationManager)
    .registerOperation<smtk::bridge::polygon::Resource, smtk::bridge::polygon::Write>();
}

void registerResources(smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->registerResource<smtk::bridge::polygon::Resource>();
}
}
}
}
