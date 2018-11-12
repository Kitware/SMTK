//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_rgg_Resource_h
#define pybind_smtk_session_rgg_Resource_h

#include <pybind11/pybind11.h>

#include "smtk/session/rgg/Resource.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::rgg::Resource, smtk::model::Resource > pybind11_init_smtk_session_rgg_Resource(py::module &m)
{
  PySharedPtrClass< smtk::session::rgg::Resource, smtk::model::Resource > instance(m, "Resource");
  instance
    .def_static("create", (std::shared_ptr<smtk::session::rgg::Resource> (*)()) &smtk::session::rgg::Resource::create)
    .def_static("create", (std::shared_ptr<smtk::session::rgg::Resource> (*)(::std::shared_ptr<smtk::session::rgg::Resource> &)) &smtk::session::rgg::Resource::create, py::arg("ref"))
    .def("session", &smtk::session::rgg::Resource::session)
    .def("setSession", &smtk::session::rgg::Resource::setSession)
    .def_static("CastTo", [](const std::shared_ptr<smtk::resource::Resource> i) {
        return std::dynamic_pointer_cast<smtk::session::rgg::Resource>(i);
      })
    ;
  return instance;
}

#endif
