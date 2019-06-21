//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_Resource_h
#define pybind_smtk_model_Resource_h

#include <pybind11/pybind11.h>

#include "smtk/session/mesh/Resource.h"
#include "smtk/session/multiscale/Resource.h"
#include "smtk/model/Resource.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::multiscale::Resource> pybind11_init_smtk_session_multiscale_Resource(py::module &m)
{
  PySharedPtrClass< smtk::session::multiscale::Resource, smtk::session::mesh::Resource > instance(m, "Resource");
  instance
    .def_static("create", (std::shared_ptr<smtk::session::multiscale::Resource> (*)()) &smtk::session::multiscale::Resource::create)
    .def_static("create", (std::shared_ptr<smtk::session::multiscale::Resource> (*)(::std::shared_ptr<smtk::session::multiscale::Resource> &)) &smtk::session::multiscale::Resource::create, py::arg("ref"))
    .def("session", &smtk::session::multiscale::Resource::session)
    .def("setSession", &smtk::session::multiscale::Resource::setSession)
    .def_static("CastTo", [](const std::shared_ptr<smtk::resource::Resource> i) {
        return std::dynamic_pointer_cast<smtk::session::multiscale::Resource>(i);
      })
    ;
  return instance;
}

#endif
