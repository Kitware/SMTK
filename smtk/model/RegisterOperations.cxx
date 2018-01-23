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
#include "smtk/model/RegisterOperations.h"

#include "smtk/model/operators/AddAuxiliaryGeometry.h"
#include "smtk/model/operators/AddImage.h"
#include "smtk/model/operators/AssignColors.h"
#include "smtk/model/operators/CloseModel.h"
#include "smtk/model/operators/CreateInstances.h"
#include "smtk/model/operators/EntityGroupOperator.h"
#include "smtk/model/operators/ExportModelJSON.h"
#include "smtk/model/operators/ExportSMTKModel.h"
#include "smtk/model/operators/GroupAuxiliaryGeometry.h"
#include "smtk/model/operators/LoadSMTKModel.h"
#include "smtk/model/operators/SaveSMTKModel.h"
#include "smtk/model/operators/SetProperty.h"
#include "smtk/model/operators/TerrainExtraction.h"

namespace smtk
{
namespace model
{

void registerOperations(smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->registerOperator<smtk::model::AddAuxiliaryGeometry>(
    "smtk::model::AddAuxiliaryGeometry");
  operationManager->registerOperator<smtk::model::AddImage>("smtk::model::AddImage");
  operationManager->registerOperator<smtk::model::AssignColors>("smtk::model::AssignColors");
  operationManager->registerOperator<smtk::model::CloseModel>("smtk::model::CloseModel");
  operationManager->registerOperator<smtk::model::CreateInstances>("smtk::model::CreateInstances");
  operationManager->registerOperator<smtk::model::EntityGroupOperator>(
    "smtk::model::EntityGroupOperator");
  operationManager->registerOperator<smtk::model::ExportModelJSON>("smtk::model::ExportModelJSON");
  operationManager->registerOperator<smtk::model::ExportSMTKModel>("smtk::model::ExportSMTKModel");
  operationManager->registerOperator<smtk::model::GroupAuxiliaryGeometry>(
    "smtk::model::GroupAuxiliaryGeometry");
  operationManager->registerOperator<smtk::model::LoadSMTKModel>("smtk::model::LoadSMTKModel");
  operationManager->registerOperator<smtk::model::SaveSMTKModel>("smtk::model::SaveSMTKModel");
  operationManager->registerOperator<smtk::model::SetProperty>("smtk::model::SetProperty");
  operationManager->registerOperator<smtk::model::TerrainExtraction>(
    "smtk::model::TerrainExtraction");
}
}
}
