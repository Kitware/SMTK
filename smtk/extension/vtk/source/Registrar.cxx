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
#include "smtk/extension/vtk/source/Registrar.h"

#include "smtk/extension/vtk/source/Backend.h"

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace source
{

void Registrar::registerTo(const smtk::geometry::Manager::Ptr& geometryManager)
{
  geometryManager->registerBackend<smtk::extension::vtk::source::Backend>();
}

void Registrar::unregisterFrom(const smtk::geometry::Manager::Ptr& geometryManager)
{
  geometryManager->unregisterBackend<smtk::extension::vtk::source::Backend>();
}
}
}
}
}
