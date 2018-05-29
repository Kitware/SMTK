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

#include "smtk/model/Manager.h"

#include "smtk/model/operators/AddAuxiliaryGeometry.h"
#include "smtk/model/operators/AddImage.h"
#include "smtk/model/operators/AssignColors.h"
#include "smtk/model/operators/CloseModel.h"
#include "smtk/model/operators/CreateInstances.h"
#include "smtk/model/operators/EntityGroupOperation.h"
#include "smtk/model/operators/ExportModelJSON.h"
#include "smtk/model/operators/GroupAuxiliaryGeometry.h"
#include "smtk/model/operators/SetProperty.h"
#include "smtk/model/operators/TerrainExtraction.h"

#include <tuple>

namespace smtk
{
namespace model
{
namespace
{
typedef std::tuple<AddAuxiliaryGeometry, AddImage, AssignColors, CloseModel, CreateInstances,
  EntityGroupOperation, ExportModelJSON, GroupAuxiliaryGeometry, SetProperty, TerrainExtraction>
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
  resourceManager->registerResource<smtk::model::Manager>();
}

void Registrar::unregisterFrom(const smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->unregisterResource<smtk::model::Manager>();
}
}
}
