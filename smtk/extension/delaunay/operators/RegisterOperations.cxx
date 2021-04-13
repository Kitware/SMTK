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
#include "smtk/extension/delaunay/operators/RegisterOperations.h"

#include "smtk/extension/delaunay/operators/TessellateFaces.h"
#include "smtk/extension/delaunay/operators/TriangulateFaces.h"

namespace smtk
{
namespace extension
{
namespace delaunay
{

void registerOperations(smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->registerOperation<smtk::extension::delaunay::TessellateFaces>(
    "smtk::extension::delaunay::TessellateFaces");
  operationManager->registerOperation<smtk::extension::delaunay::TriangulateFaces>(
    "smtk::extension::delaunay::TriangulateFaces");
}
} // namespace delaunay
} // namespace extension
} // namespace smtk
