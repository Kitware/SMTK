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
#include "smtk/extension/vtk/markup/Registrar.h"

#include "smtk/extension/vtk/markup/RegisterVTKBackend.h"

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace markup
{
void Registrar::registerTo(const smtk::geometry::Manager::Ptr& geometryManager)
{
  geometryManager->registerBackend<smtk::extension::vtk::geometry::Backend>();
  RegisterVTKBackend::registerClass();
}

void Registrar::unregisterFrom(const smtk::geometry::Manager::Ptr& geometryManager)
{
  geometryManager->unregisterBackend<smtk::extension::vtk::geometry::Backend>();
}
} // namespace markup
} // namespace vtk
} // namespace extension
} // namespace smtk
