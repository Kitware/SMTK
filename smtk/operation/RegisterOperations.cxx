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
#include "smtk/operation/ImportPythonOperation.h"
#include "smtk/operation/LoadResource.h"
#include "smtk/operation/SaveResource.h"

namespace smtk
{
namespace operation
{

void registerOperations(smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->registerOperation<smtk::operation::ImportPythonOperation>(
    "smtk::operation::ImportPythonOperation");
  operationManager->registerOperation<smtk::operation::CreateResource>(
    "smtk::operation::CreateResource");
  operationManager->registerOperation<smtk::operation::LoadResource>(
    "smtk::operation::LoadResource");
  operationManager->registerOperation<smtk::operation::SaveResource>(
    "smtk::operation::SaveResource");
}
}
}
