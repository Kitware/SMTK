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
#include "smtk/session/opencascade/vtk/Registrar.h"

#include "smtk/session/opencascade/vtk/Geometry.h"

#include "smtk/session/opencascade/Resource.h"

#include "smtk/extension/vtk/geometry/Backend.h"

#include "smtk/geometry/Generator.h"
#include "smtk/geometry/Manager.h"

namespace smtk
{
namespace session
{
namespace opencascade
{
namespace vtk
{
namespace
{
class RegisterOpencascadeVTKBackend : public smtk::geometry::Supplier<RegisterOpencascadeVTKBackend>
{
public:
  bool valid(const Specification& in) const override
  {
    smtk::extension::vtk::geometry::Backend backend;
    return std::get<1>(in).index() == backend.index();
  }

  GeometryPtr operator()(const Specification& in) override
  {
    auto rsrc = std::dynamic_pointer_cast<smtk::session::opencascade::Resource>(std::get<0>(in));
    if (rsrc)
    {
      auto provider = new Geometry(rsrc);
      return GeometryPtr(provider);
    }
    throw std::invalid_argument("Not a opencascade resource.");
    return nullptr;
  }
};
}

void Registrar::registerTo(const smtk::resource::Manager::Ptr& resourceManager)
{
  (void)resourceManager;
  RegisterOpencascadeVTKBackend::registerClass();
}

void Registrar::unregisterFrom(const smtk::resource::Manager::Ptr&)
{
}
}
}
}
}
