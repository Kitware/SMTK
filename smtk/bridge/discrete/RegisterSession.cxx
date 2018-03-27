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
#include "smtk/bridge/discrete/RegisterSession.h"

#include "smtk/bridge/discrete/operators/CreateEdgesOperation.h"
#include "smtk/bridge/discrete/operators/EdgeOperation.h"
#include "smtk/bridge/discrete/operators/EntityGroupOperation.h"
#include "smtk/bridge/discrete/operators/GrowOperation.h"
#include "smtk/bridge/discrete/operators/ImportOperation.h"
#include "smtk/bridge/discrete/operators/MergeOperation.h"
#include "smtk/bridge/discrete/operators/ReadOperation.h"
#include "smtk/bridge/discrete/operators/ReadResource.h"
#include "smtk/bridge/discrete/operators/RemoveModel.h"
#include "smtk/bridge/discrete/operators/SetProperty.h"
#include "smtk/bridge/discrete/operators/SplitFaceOperation.h"
#include "smtk/bridge/discrete/operators/WriteOperation.h"
#include "smtk/bridge/discrete/operators/WriteResource.h"

#include "smtk/bridge/discrete/Resource.h"

#include "smtk/operation/groups/ImporterGroup.h"
#include "smtk/operation/groups/ReaderGroup.h"
#include "smtk/operation/groups/WriterGroup.h"

namespace smtk
{
namespace bridge
{
namespace discrete
{

typedef std::tuple<CreateEdgesOperation, EdgeOperation, EntityGroupOperation, GrowOperation,
  ImportOperation, MergeOperation, ReadOperation, ReadResource, RemoveModel, SetProperty,
  SplitFaceOperation, WriteOperation, WriteResource>
  OperationList;

void registerOperations(smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->registerOperations<OperationList>();

  smtk::operation::ImporterGroup(operationManager)
    .registerOperation<smtk::bridge::discrete::Resource, smtk::bridge::discrete::ImportOperation>();

  smtk::operation::ReaderGroup(operationManager)
    .registerOperation<smtk::bridge::discrete::Resource, smtk::bridge::discrete::ReadResource>();

  // When resources were introduced, the JSON description for a discrete model
  // changed from "discrete" to "smtk::bridge::discrete::Resource". This functor
  // enables reading a legacy file with the JSON tag "discrete".
  smtk::operation::ReaderGroup(operationManager)
    .registerOperation(smtk::common::typeName<ReadResource>(), "discrete");

  smtk::operation::WriterGroup(operationManager)
    .registerOperation<smtk::bridge::discrete::Resource, smtk::bridge::discrete::WriteResource>();
}

void registerResources(smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->registerResource<smtk::bridge::discrete::Resource>(readResource, writeResource);

  // When resources were introduced, the JSON description for a discrete model
  // changed from "discrete" to "smtk::bridge::discrete::Resource". This functor
  // enables reading a legacy file with the JSON tag "discrete".
  resourceManager->addLegacyReader("discrete", readResource);
}

void unregisterOperations(smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->unregisterOperations<OperationList>();
}

void unregisterResources(smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->unregisterResource<smtk::bridge::discrete::Resource>();
}
}
}
}
