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
#include "smtk/session/polygon/Registrar.h"

#include "smtk/session/polygon/operators/CleanGeometry.h"
#include "smtk/session/polygon/operators/CreateEdge.h"
#include "smtk/session/polygon/operators/CreateEdgeFromPoints.h"
#include "smtk/session/polygon/operators/CreateEdgeFromVertices.h"
#include "smtk/session/polygon/operators/CreateFaces.h"
#include "smtk/session/polygon/operators/CreateFacesFromEdges.h"
#include "smtk/session/polygon/operators/CreateModel.h"
#include "smtk/session/polygon/operators/CreateVertices.h"
#include "smtk/session/polygon/operators/Delete.h"
#include "smtk/session/polygon/operators/DemoteVertex.h"
#include "smtk/session/polygon/operators/ForceCreateFace.h"
#include "smtk/session/polygon/operators/ImportPPG.h"
#include "smtk/session/polygon/operators/LegacyRead.h"
#include "smtk/session/polygon/operators/Read.h"
#include "smtk/session/polygon/operators/SplitEdge.h"
#include "smtk/session/polygon/operators/TweakEdge.h"
#include "smtk/session/polygon/operators/Write.h"

#include "smtk/session/polygon/Resource.h"

#include "smtk/operation/groups/CreatorGroup.h"
#include "smtk/operation/groups/ReaderGroup.h"
#include "smtk/operation/groups/WriterGroup.h"

#ifdef VTK_SUPPORT
#include "smtk/operation/groups/ImporterGroup.h"
#include "smtk/session/polygon/operators/ExtractContours.h"
#include "smtk/session/polygon/operators/Import.h"
#endif

namespace smtk
{
namespace session
{
namespace polygon
{

namespace
{
typedef std::tuple<
  CleanGeometry,
  CreateEdge,
  CreateEdgeFromPoints,
  CreateEdgeFromVertices,
  CreateFaces,
  CreateFacesFromEdges,
  CreateModel,
  CreateVertices,
  Delete,
  DemoteVertex,
  ForceCreateFace,
#ifdef VTK_SUPPORT
  ExtractContours,
  Import,
#endif
  ImportPPG,
  LegacyRead,
  Read,
  SplitEdge,
  TweakEdge,
  Write>
  OperationList;
} // namespace

void Registrar::registerTo(const smtk::resource::Manager::Ptr& resourceManager)
{
  smtk::string::Token dummy("smtk::session::polygon::Resource");
  resourceManager->registerResource<smtk::session::polygon::Resource>(read, write);
}

void Registrar::registerTo(const smtk::operation::Manager::Ptr& operationManager)
{
  // Register operations
  operationManager->registerOperations<OperationList>();

  smtk::operation::CreatorGroup(operationManager)
    .registerOperation<smtk::session::polygon::Resource, smtk::session::polygon::CreateModel>();
  smtk::operation::CreatorGroup(operationManager)
    .registerOperation<smtk::session::polygon::Resource, smtk::session::polygon::ImportPPG>();

#ifdef VTK_SUPPORT
  smtk::operation::ImporterGroup(operationManager)
    .registerOperation<smtk::session::polygon::Resource, smtk::session::polygon::Import>();
#endif

  smtk::operation::ReaderGroup(operationManager)
    .registerOperation<smtk::session::polygon::Resource, smtk::session::polygon::Read>();
  smtk::operation::ReaderGroup(operationManager)
    .registerOperation(smtk::common::typeName<smtk::session::polygon::LegacyRead>(), "polygon");

  smtk::operation::WriterGroup(operationManager)
    .registerOperation<smtk::session::polygon::Resource, smtk::session::polygon::Write>();
}

void Registrar::unregisterFrom(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->unregisterResource<smtk::session::polygon::Resource>();
}

void Registrar::unregisterFrom(const smtk::operation::Manager::Ptr& operationManager)
{
  smtk::operation::CreatorGroup(operationManager)
    .unregisterOperation<smtk::session::polygon::CreateModel>();
  smtk::operation::CreatorGroup(operationManager)
    .unregisterOperation<smtk::session::polygon::ImportPPG>();

#ifdef VTK_SUPPORT
  smtk::operation::ImporterGroup(operationManager)
    .unregisterOperation<smtk::session::polygon::Import>();
#endif

  smtk::operation::ReaderGroup(operationManager)
    .unregisterOperation<smtk::session::polygon::Read>();
  smtk::operation::ReaderGroup(operationManager)
    .unregisterOperation<smtk::session::polygon::LegacyRead>();

  smtk::operation::WriterGroup(operationManager)
    .unregisterOperation<smtk::session::polygon::Write>();

  operationManager->unregisterOperations<OperationList>();
}
} // namespace polygon
} // namespace session
} // namespace smtk
