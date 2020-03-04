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
#include "smtk/extension/vtk/mesh/Registrar.h"

#include "smtk/extension/vtk/source/Backend.h"

#include "smtk/extension/vtk/mesh/Geometry.h"

#include "smtk/geometry/Generator.h"

#include "smtk/mesh/core/Resource.h"

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace mesh
{

namespace
{
class RegisterVTKBackend : public smtk::geometry::Supplier<RegisterVTKBackend>
{
public:
  bool valid(const smtk::geometry::Specification& in) const override
  {
    smtk::extension::vtk::source::Backend backend;
    return std::get<1>(in).index() == backend.index();
  }

  GeometryPtr operator()(const smtk::geometry::Specification& in) override
  {
    auto rsrc = std::dynamic_pointer_cast<smtk::mesh::Resource>(std::get<0>(in));
    if (rsrc)
    {
      auto provider = new Geometry(rsrc);
      return GeometryPtr(provider);
    }
    throw std::invalid_argument("Not a mesh resource.");
    return nullptr;
  }
};
}

void Registrar::registerTo(const smtk::geometry::Manager::Ptr& geometryManager)
{
  geometryManager->registerBackend<smtk::extension::vtk::source::Backend>();
  RegisterVTKBackend::registerClass();
}

void Registrar::unregisterFrom(const smtk::geometry::Manager::Ptr& geometryManager)
{
  geometryManager->unregisterBackend<smtk::extension::vtk::source::Backend>();
}
}
}
}
}
