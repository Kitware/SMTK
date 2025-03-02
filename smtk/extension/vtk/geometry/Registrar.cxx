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

#include "smtk/extension/vtk/geometry/BoundingBox.h"
#include "smtk/extension/vtk/geometry/ClosestPoint.h"
#include "smtk/extension/vtk/geometry/DistanceTo.h"

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace geometry
{
namespace
{
typedef std::tuple<BoundingBox, /*ClosestPoint,*/ DistanceTo> QueryList;
}

void Registrar::registerTo(const smtk::geometry::Manager::Ptr& geometryManager)
{
  geometryManager->registerBackend<smtk::extension::vtk::geometry::Backend>();
}

void Registrar::unregisterFrom(const smtk::geometry::Manager::Ptr& geometryManager)
{
  geometryManager->unregisterBackend<smtk::extension::vtk::geometry::Backend>();
}

void Registrar::registerTo(const smtk::resource::query::Manager::Ptr& queryManager)
{
  queryManager->registerQueriesIf<QueryList>([](smtk::resource::Resource& resource) -> bool {
    if (auto* geometryResource = dynamic_cast<smtk::geometry::Resource*>(&resource))
    {
      smtk::extension::vtk::geometry::Backend vtk;
      return !!geometryResource->geometry(vtk);
    }
    return false;
  });
}

void Registrar::unregisterFrom(const smtk::resource::query::Manager::Ptr& queryManager)
{
  queryManager->unregisterQueries<QueryList>();
}
} // namespace geometry
} // namespace vtk
} // namespace extension
} // namespace smtk
