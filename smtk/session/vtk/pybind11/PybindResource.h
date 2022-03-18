//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_vtk_Resource_h
#define pybind_smtk_session_vtk_Resource_h

#include <pybind11/pybind11.h>

#include "smtk/session/vtk/Resource.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::session::vtk::Resource> pybind11_init_smtk_session_vtk_Resource(py::module &m)
{
  PySharedPtrClass< smtk::session::vtk::Resource, smtk::model::Resource > instance(m, "Resource");
  instance
    .def_static("create", (std::shared_ptr<smtk::session::vtk::Resource> (*)()) &smtk::session::vtk::Resource::create)
    .def_static("create", (std::shared_ptr<smtk::session::vtk::Resource> (*)(::std::shared_ptr<smtk::session::vtk::Resource> &)) &smtk::session::vtk::Resource::create, py::arg("ref"))
    .def("session", &smtk::session::vtk::Resource::session)
    .def("setSession", &smtk::session::vtk::Resource::setSession)
    .def_static("CastTo", [](const std::shared_ptr<smtk::resource::Resource> i) {
        return std::dynamic_pointer_cast<smtk::session::vtk::Resource>(i);
      })
    ;
  return instance;
}

#endif
