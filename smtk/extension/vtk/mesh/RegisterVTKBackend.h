//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_vtk_mesh_RegisterVTKBackend_h
#define smtk_extension_vtk_mesh_RegisterVTKBackend_h
#ifndef __VTK_WRAP__

#include "smtk/extension/vtk/mesh/vtkSMTKMeshExtModule.h"

#include "smtk/extension/vtk/geometry/Backend.h"
#include "smtk/extension/vtk/geometry/Registrar.h"
#include "smtk/extension/vtk/mesh/Geometry.h"
#include "smtk/geometry/Generator.h"
#include "smtk/geometry/Manager.h"
#include "smtk/mesh/core/Resource.h"
#include "smtk/mesh/resource/Registrar.h"

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace mesh
{

class VTKSMTKMESHEXT_EXPORT RegisterVTKBackend : public smtk::geometry::Supplier<RegisterVTKBackend>
{
public:
  bool valid(const smtk::geometry::Specification& in) const override
  {
    smtk::extension::vtk::geometry::Backend backend;
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
}
}
}

#endif // __VTK_WRAP__
#endif // smtk_extension_vtk_mesh_RegisterVTKBackend_h
