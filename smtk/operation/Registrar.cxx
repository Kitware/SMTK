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
#include "smtk/operation/Registrar.h"

#ifdef SMTK_PYTHON_ENABLED
#include "smtk/operation/operators/ImportPythonOperation.h"
#endif
#include "smtk/operation/operators/ImportResource.h"
#include "smtk/operation/operators/MarkModified.h"
#include "smtk/operation/operators/ReadResource.h"
#include "smtk/operation/operators/RemoveResource.h"
#include "smtk/operation/operators/WriteResource.h"

#include <tuple>

namespace smtk
{
namespace operation
{
namespace
{
typedef std::tuple<
#ifdef SMTK_PYTHON_ENABLED
  ImportPythonOperation,
#endif
  ImportResource, MarkModified, ReadResource, RemoveResource, WriteResource>
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
}
}
