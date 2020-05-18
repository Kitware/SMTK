//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_session_vtk_RegisterVTKBackend_h
#define smtk_session_vtk_RegisterVTKBackend_h

#include "smtk/session/vtk/Exports.h"

#include "smtk/extension/vtk/geometry/Backend.h"

#include "smtk/geometry/Generator.h"

#include "smtk/session/vtk/Geometry.h"

namespace smtk
{
namespace session
{
namespace vtk
{

class SMTKVTKSESSION_EXPORT RegisterVTKBackend : public smtk::geometry::Supplier<RegisterVTKBackend>
{
public:
  bool valid(const Specification& in) const override
  {
    smtk::extension::vtk::geometry::Backend backend;
    return std::get<1>(in).index() == backend.index();
  }

  GeometryPtr operator()(const Specification& in) override
  {
    auto rsrc = std::dynamic_pointer_cast<smtk::session::vtk::Resource>(std::get<0>(in));
    if (rsrc)
    {
      auto provider = new Geometry(rsrc);
      return GeometryPtr(provider);
    }
    throw std::invalid_argument("Not a VTK-session resource.");
    return nullptr;
  }
};
} // namespace vtk
} // namespace session
} // namespace smtk

#endif // smtk_session_vtk_RegisterVTKBackend_h
