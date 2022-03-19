//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_mesh_Resource_h
#define pybind_smtk_session_mesh_Resource_h

#include <pybind11/pybind11.h>

#include "smtk/session/mesh/Resource.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::session::mesh::Resource> pybind11_init_smtk_session_mesh_Resource(py::module &m)
{
  PySharedPtrClass< smtk::session::mesh::Resource, smtk::model::Resource > instance(m, "Resource");
  instance
    .def_static("create", (std::shared_ptr<smtk::session::mesh::Resource> (*)()) &smtk::session::mesh::Resource::create)
    .def_static("create", (std::shared_ptr<smtk::session::mesh::Resource> (*)(::std::shared_ptr<smtk::session::mesh::Resource> &)) &smtk::session::mesh::Resource::create, py::arg("ref"))
    .def("session", &smtk::session::mesh::Resource::session)
    .def("setSession", &smtk::session::mesh::Resource::setSession)
    .def("resource", &smtk::session::mesh::Resource::resource)
    .def_static("CastTo", [](const std::shared_ptr<smtk::resource::Resource> i) {
        return std::dynamic_pointer_cast<smtk::session::mesh::Resource>(i);
      })
    ;
  return instance;
}

#endif
