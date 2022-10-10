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
#include "smtk/model/Registrar.h"

#include "smtk/model/Resource.h"

#include "smtk/view/OperationIcons.h"

#include "smtk/model/operators/AddAuxiliaryGeometry.h"
#include "smtk/model/operators/AddImage.h"
#include "smtk/model/operators/CloseModel.h"
#include "smtk/model/operators/CreateInstances.h"
#include "smtk/model/operators/Delete.h"
#include "smtk/model/operators/DivideInstance.h"
#include "smtk/model/operators/EntityGroupOperation.h"
#include "smtk/model/operators/ExportModelJSON.h"
#include "smtk/model/operators/GroupAuxiliaryGeometry.h"
#include "smtk/model/operators/MergeInstances.h"
#include "smtk/model/operators/SetInstancePrototype.h"
#include "smtk/model/operators/TerrainExtraction.h"

#include "smtk/operation/groups/DeleterGroup.h"
#include "smtk/operation/groups/InternalGroup.h"

#include <tuple>

namespace smtk
{
namespace model
{
namespace
{
typedef std::tuple<
  AddAuxiliaryGeometry,
  AddImage,
  CloseModel,
  CreateInstances,
  Delete,
  DivideInstance,
  EntityGroupOperation,
  ExportModelJSON,
  GroupAuxiliaryGeometry,
  MergeInstances,
  SetInstancePrototype,
  TerrainExtraction>
  OperationList;
}

void Registrar::registerTo(const smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->registerOperations<OperationList>();

  smtk::operation::InternalGroup internalGroup(operationManager);
  internalGroup.registerOperation<Delete>();
  smtk::operation::DeleterGroup deleters(operationManager);
  deleters.registerOperation<Delete>();
}

void Registrar::unregisterFrom(const smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->unregisterOperations<OperationList>();

  smtk::operation::InternalGroup internalGroup(operationManager);
  internalGroup.unregisterOperation<Delete>();
  smtk::operation::DeleterGroup deleters(operationManager);
  deleters.unregisterOperation<Delete>();
}

void Registrar::registerTo(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->registerResource<smtk::model::Resource>();
}

void Registrar::unregisterFrom(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->unregisterResource<smtk::model::Resource>();
}

} // namespace model
} // namespace smtk
