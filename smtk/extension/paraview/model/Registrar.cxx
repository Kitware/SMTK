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
#include "smtk/extension/paraview/model/Registrar.h"

#include "smtk/extension/paraview/model/VTKModelInstancePlacementSelection.h"

#include "smtk/extension/paraview/server/RespondToVTKSelection.h"
#include "smtk/extension/paraview/server/VTKSelectionResponderGroup.h"

#include "smtk/operation/groups/InternalGroup.h"

#include "smtk/model/Resource.h"

namespace smtk
{
namespace extension
{
namespace paraview
{
namespace model
{

namespace
{
using OperationList = std::tuple<smtk::view::VTKModelInstancePlacementSelection>;
}

void Registrar::registerTo(const smtk::operation::Manager::Ptr& operationManager)
{
  // Register operations
  operationManager->registerOperations<OperationList>();

  smtk::view::VTKSelectionResponderGroup responders(operationManager, nullptr);
  responders
    .registerOperation<smtk::model::Resource, smtk::view::VTKModelInstancePlacementSelection>();

  smtk::operation::InternalGroup internal(operationManager);
  internal.registerOperation<smtk::view::VTKModelInstancePlacementSelection>();
}

void Registrar::unregisterFrom(const smtk::operation::Manager::Ptr& operationManager)
{
  smtk::view::VTKSelectionResponderGroup responders(operationManager, nullptr);
  responders.unregisterOperation<smtk::view::VTKModelInstancePlacementSelection>();

  smtk::operation::InternalGroup internal(operationManager);
  internal.unregisterOperation<smtk::view::VTKModelInstancePlacementSelection>();

  operationManager->unregisterOperations<OperationList>();
}
} // namespace model
} // namespace paraview
} // namespace extension
} // namespace smtk
