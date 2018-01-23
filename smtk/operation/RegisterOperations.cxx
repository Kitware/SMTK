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
#include "smtk/operation/RegisterOperations.h"

#include "smtk/operation/CreateResource.h"
#include "smtk/operation/ImportPythonOperator.h"
#include "smtk/operation/LoadResource.h"
#include "smtk/operation/SaveResource.h"

namespace smtk
{
namespace operation
{

void registerOperations(smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->registerOperator<smtk::operation::ImportPythonOperator>(
    "smtk::operation::ImportPythonOperator");
  operationManager->registerOperator<smtk::operation::CreateResource>(
    "smtk::operation::CreateResource");
  operationManager->registerOperator<smtk::operation::LoadResource>(
    "smtk::operation::LoadResource");
  operationManager->registerOperator<smtk::operation::SaveResource>(
    "smtk::operation::SaveResource");
}
}
}
