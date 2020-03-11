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
#include "smtk/extension/vtk/geometry/Registrar.h"

#include "smtk/extension/vtk/geometry/Backend.h"

#include "smtk/geometry/Generator.h"

#include "smtk/mesh/core/Resource.h"

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace geometry
{
void Registrar::registerTo(const smtk::geometry::Manager::Ptr& geometryManager)
{
  geometryManager->registerBackend<smtk::extension::vtk::geometry::Backend>();
}

void Registrar::unregisterFrom(const smtk::geometry::Manager::Ptr& geometryManager)
{
  geometryManager->unregisterBackend<smtk::extension::vtk::geometry::Backend>();
}
}
}
}
}
