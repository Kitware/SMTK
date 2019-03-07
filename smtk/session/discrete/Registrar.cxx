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
#include "smtk/session/discrete/Registrar.h"

#include "smtk/session/discrete/operators/CreateEdgesOperation.h"
#include "smtk/session/discrete/operators/EdgeOperation.h"
#include "smtk/session/discrete/operators/EntityGroupOperation.h"
#include "smtk/session/discrete/operators/GrowOperation.h"
#include "smtk/session/discrete/operators/ImportOperation.h"
#include "smtk/session/discrete/operators/LegacyReadResource.h"
#include "smtk/session/discrete/operators/MergeOperation.h"
#include "smtk/session/discrete/operators/ReadOperation.h"
#include "smtk/session/discrete/operators/ReadResource.h"
#include "smtk/session/discrete/operators/RemoveModel.h"
#include "smtk/session/discrete/operators/SetProperty.h"
#include "smtk/session/discrete/operators/SplitFaceOperation.h"
#include "smtk/session/discrete/operators/WriteOperation.h"
#include "smtk/session/discrete/operators/WriteResource.h"

#include "smtk/session/discrete/Resource.h"

#include "smtk/operation/groups/ImporterGroup.h"
#include "smtk/operation/groups/ReaderGroup.h"
#include "smtk/operation/groups/WriterGroup.h"

namespace smtk
{
namespace session
{
namespace discrete
{

namespace
{
typedef std::tuple<CreateEdgesOperation, EdgeOperation, EntityGroupOperation, GrowOperation,
  ImportOperation, LegacyReadResource, MergeOperation, ReadOperation, ReadResource, RemoveModel,
  SetProperty, SplitFaceOperation, WriteOperation, WriteResource>
  OperationList;
}

void Registrar::registerTo(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->registerResource<smtk::session::discrete::Resource>(readResource, writeResource);

  // When moving from CJSON to nlohmann::json, the file format for discrete
  // models changed slightly. This call facilitates reading the old format
  // using our new tools.
  resourceManager->addLegacyReader("discrete", legacyReadResource);
}

void Registrar::registerTo(const smtk::operation::Manager::Ptr& operationManager)
{
  // Register operations
  operationManager->registerOperations<OperationList>();

  smtk::operation::ImporterGroup(operationManager)
    .registerOperation<smtk::session::discrete::Resource,
      smtk::session::discrete::ImportOperation>();
  smtk::operation::ImporterGroup(operationManager)
    .registerOperation<smtk::session::discrete::Resource, smtk::session::discrete::ReadOperation>();

  smtk::operation::ReaderGroup(operationManager)
    .registerOperation<smtk::session::discrete::Resource, smtk::session::discrete::ReadResource>();
  smtk::operation::ReaderGroup(operationManager)
    .registerOperation(smtk::common::typeName<LegacyReadResource>(), "discrete");

  smtk::operation::WriterGroup(operationManager)
    .registerOperation<smtk::session::discrete::Resource, smtk::session::discrete::WriteResource>();
}

void Registrar::unregisterFrom(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->unregisterResource<smtk::session::discrete::Resource>();
}

void Registrar::unregisterFrom(const smtk::operation::Manager::Ptr& operationManager)
{
  smtk::operation::ImporterGroup(operationManager)
    .unregisterOperation<smtk::session::discrete::ImportOperation>();
  smtk::operation::ImporterGroup(operationManager)
    .unregisterOperation<smtk::session::discrete::ReadOperation>();

  smtk::operation::ReaderGroup(operationManager)
    .unregisterOperation<smtk::session::discrete::ReadResource>();
  smtk::operation::ReaderGroup(operationManager).unregisterOperation<LegacyReadResource>();

  smtk::operation::WriterGroup(operationManager)
    .unregisterOperation<smtk::session::discrete::WriteResource>();

  operationManager->unregisterOperations<OperationList>();
}
}
}
}
