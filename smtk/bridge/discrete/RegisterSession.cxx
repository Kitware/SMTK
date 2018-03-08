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

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"

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

#include "smtk/model/SessionIOJSON.h"

#include "smtk/operation/groups/ImporterGroup.h"
#include "smtk/operation/groups/ReaderGroup.h"
#include "smtk/operation/groups/WriterGroup.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "boost/filesystem.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

using namespace boost::filesystem;

namespace smtk
{
namespace bridge
{
namespace discrete
{

void registerOperations(smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->registerOperation<smtk::bridge::discrete::CreateEdgesOperation>(
    "smtk::bridge::discrete::CreateEdgesOperation");
  operationManager->registerOperation<smtk::bridge::discrete::EdgeOperation>(
    "smtk::bridge::discrete::EdgeOperation");
  operationManager->registerOperation<smtk::bridge::discrete::EntityGroupOperation>(
    "smtk::bridge::discrete::EntityGroupOperation");
  operationManager->registerOperation<smtk::bridge::discrete::GrowOperation>(
    "smtk::bridge::discrete::GrowOperation");
  operationManager->registerOperation<smtk::bridge::discrete::ImportOperation>(
    "smtk::bridge::discrete::ImportOperation");
  operationManager->registerOperation<smtk::bridge::discrete::MergeOperation>(
    "smtk::bridge::discrete::MergeOperation");
  operationManager->registerOperation<smtk::bridge::discrete::ReadOperation>(
    "smtk::bridge::discrete::ReadOperation");
  operationManager->registerOperation<smtk::bridge::discrete::ReadResource>(
    "smtk::bridge::discrete::ReadResource");
  operationManager->registerOperation<smtk::bridge::discrete::RemoveModel>(
    "smtk::bridge::discrete::RemoveModel");
  operationManager->registerOperation<smtk::bridge::discrete::SetProperty>(
    "smtk::bridge::discrete::SetProperty");
  operationManager->registerOperation<smtk::bridge::discrete::SplitFaceOperation>(
    "smtk::bridge::discrete::SplitFaceOperation");
  operationManager->registerOperation<smtk::bridge::discrete::WriteOperation>(
    "smtk::bridge::discrete::WriteOperation");
  operationManager->registerOperation<smtk::bridge::discrete::WriteResource>(
    "smtk::bridge::discrete::WriteResource");

  smtk::operation::ImporterGroup(operationManager)
    .registerOperation<smtk::bridge::discrete::Resource, smtk::bridge::discrete::ImportOperation>();

  smtk::operation::ReaderGroup(operationManager)
    .registerOperation<smtk::bridge::discrete::Resource, smtk::bridge::discrete::ReadResource>();

  // When resources were introduced, the JSON description for a discrete model
  // changed from "discrete" to "discrete model". This functor enables reading a
  // legacy file with the JSON tag "discrete".
  smtk::operation::ReaderGroup(operationManager)
    .registerOperation("smtk::bridge::discrete::ReadResource", "discrete");

  smtk::operation::WriterGroup(operationManager)
    .registerOperation<smtk::bridge::discrete::Resource, smtk::bridge::discrete::WriteResource>();
}

void registerResources(smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->registerResource<smtk::bridge::discrete::Resource>();
}
}
}
}
