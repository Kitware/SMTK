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
#include "smtk/extension/paraview/server/Registrar.h"

#include "smtk/extension/paraview/server/RespondToVTKSelection.h"
#include "smtk/extension/paraview/server/VTKSelectionResponderGroup.h"

#include "smtk/operation/groups/InternalGroup.h"

namespace smtk
{
namespace extension
{
namespace paraview
{
namespace server
{

namespace
{
using OperationList = std::tuple<smtk::view::RespondToVTKSelection>;
}

void Registrar::registerTo(const smtk::operation::Manager::Ptr& operationManager)
{
  // Register operations
  operationManager->registerOperations<OperationList>();

  smtk::view::VTKSelectionResponderGroup responders(operationManager, nullptr);
  responders.registerOperation<smtk::resource::Resource, smtk::view::RespondToVTKSelection>();

  smtk::operation::InternalGroup internal(operationManager);
  internal.registerOperation<smtk::view::RespondToVTKSelection>();
}

void Registrar::unregisterFrom(const smtk::operation::Manager::Ptr& operationManager)
{
  smtk::view::VTKSelectionResponderGroup responders(operationManager, nullptr);
  responders.unregisterOperation<smtk::view::RespondToVTKSelection>();

  smtk::operation::InternalGroup internal(operationManager);
  internal.unregisterOperation<smtk::view::RespondToVTKSelection>();

  operationManager->unregisterOperations<OperationList>();
}
} // namespace server
} // namespace paraview
} // namespace extension
} // namespace smtk
