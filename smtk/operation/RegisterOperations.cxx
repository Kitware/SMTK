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

#include <tuple>

namespace smtk
{
namespace operation
{

typedef std::tuple<ImportPythonOperation, ImportResource, ReadResource, WriteResource>
  OperationList;

void registerOperations(smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->registerOperations<OperationList>();
}

void unregisterOperations(smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->unregisterOperations<OperationList>();
}
}
}
