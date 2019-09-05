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
#include "smtk/extension/paraview/server/VTKMeshCellSelection.h"
#include "smtk/extension/paraview/server/VTKSelectionResponderGroup.h"

#include "smtk/operation/groups/InternalGroup.h"

#include "smtk/mesh/core/Resource.h"
#include "smtk/session/mesh/Resource.h"

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
typedef std::tuple<smtk::view::RespondToVTKSelection, smtk::view::VTKMeshCellSelection>
  OperationList;
}

void Registrar::registerTo(const smtk::operation::Manager::Ptr& operationManager)
{
  // Register operations
  operationManager->registerOperations<OperationList>();

  smtk::view::VTKSelectionResponderGroup responders(operationManager, nullptr);
  responders.registerOperation<smtk::resource::Resource, smtk::view::RespondToVTKSelection>();
  // responders.registerOperation<smtk::mesh::Resource, smtk::view::VTKMeshCellSelection>();
  // responders.registerOperation<smtk::session::mesh::Resource, smtk::view::VTKMeshCellSelection>();

  smtk::operation::InternalGroup internal(operationManager);
  internal.registerOperation<smtk::view::RespondToVTKSelection>();
  internal.registerOperation<smtk::view::VTKMeshCellSelection>();
}

void Registrar::unregisterFrom(const smtk::operation::Manager::Ptr& operationManager)
{
  smtk::view::VTKSelectionResponderGroup responders(operationManager, nullptr);
  responders.unregisterOperation<smtk::view::RespondToVTKSelection>();
  // responders.unregisterOperation<smtk::view::VTKMeshCellSelection>();

  smtk::operation::InternalGroup internal(operationManager);
  internal.unregisterOperation<smtk::view::RespondToVTKSelection>();
  internal.unregisterOperation<smtk::view::VTKMeshCellSelection>();

  operationManager->unregisterOperations<OperationList>();
}
}
}
}
}
