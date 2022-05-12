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
#include "smtk/extension/paraview/markup/Registrar.h"

// #include "smtk/extension/paraview/markup/VTKMarkupCellSelection.h"

#include "smtk/extension/paraview/server/VTKSelectionResponderGroup.h"

#include "smtk/operation/groups/InternalGroup.h"

#include "smtk/markup/Resource.h"

namespace smtk
{
namespace extension
{
namespace paraview
{
namespace markup
{

void Registrar::registerTo(const smtk::operation::Manager::Ptr& operationManager)
{
  (void)operationManager;
  // smtk::view::VTKSelectionResponderGroup responders(operationManager, nullptr);
  // responders.registerOperation<smtk::markup::Resource, smtk::view::VTKMarkupCellSelection>();

  // smtk::operation::InternalGroup internal(operationManager);
  // internal.registerOperation<smtk::view::VTKMarkupCellSelection>();
}

void Registrar::unregisterFrom(const smtk::operation::Manager::Ptr& operationManager)
{
  (void)operationManager;
  // smtk::view::VTKSelectionResponderGroup responders(operationManager, nullptr);
  // responders.unregisterOperation<smtk::view::VTKMarkupCellSelection>();

  // smtk::operation::InternalGroup internal(operationManager);
  // internal.unregisterOperation<smtk::view::VTKMarkupCellSelection>();

  // operationManager->unregisterOperations<OperationList>();
}
} // namespace markup
} // namespace paraview
} // namespace extension
} // namespace smtk
