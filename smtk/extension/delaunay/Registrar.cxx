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
#include "smtk/extension/delaunay/Registrar.h"

#include "smtk/extension/delaunay/operators/TessellateFaces.h"
#include "smtk/extension/delaunay/operators/TriangulateFaces.h"

namespace smtk
{
namespace extension
{
namespace delaunay
{

namespace
{
typedef std::tuple<TessellateFaces, TriangulateFaces> OperationList;
}

void Registrar::registerTo(const smtk::operation::Manager::Ptr& operationManager)
{
  // Register operations
  operationManager->registerOperations<OperationList>();
}

void Registrar::unregisterFrom(const smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->unregisterOperations<OperationList>();
}
} // namespace delaunay
} // namespace extension
} // namespace smtk
