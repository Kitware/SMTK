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

#include "smtk/operation/operators/ImportPythonOperation.h"
#include "smtk/operation/operators/ImportResource.h"
#include "smtk/operation/operators/ReadResource.h"
#include "smtk/operation/operators/WriteResource.h"

namespace smtk
{
namespace operation
{

void registerOperations(smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->registerOperation<smtk::operation::ImportPythonOperation>(
    "smtk::operation::ImportPythonOperation");
  operationManager->registerOperation<smtk::operation::ImportResource>(
    "smtk::operation::ImportResource");
  operationManager->registerOperation<smtk::operation::ReadResource>(
    "smtk::operation::ReadResource");
  operationManager->registerOperation<smtk::operation::WriteResource>(
    "smtk::operation::WriteResource");
}
}
}
