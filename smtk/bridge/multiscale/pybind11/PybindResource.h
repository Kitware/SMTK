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

#include "smtk/bridge/mesh/Resource.h"
#include "smtk/bridge/multiscale/Resource.h"
#include "smtk/model/Resource.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::multiscale::Resource, smtk::model::Resource > pybind11_init_smtk_bridge_multiscale_Resource(py::module &m)
{
  PySharedPtrClass< smtk::bridge::multiscale::Resource, smtk::model::Resource > instance(m, "Resource");
  instance
    .def_static("create", (std::shared_ptr<smtk::bridge::multiscale::Resource> (*)()) &smtk::bridge::multiscale::Resource::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::multiscale::Resource> (*)(::std::shared_ptr<smtk::bridge::multiscale::Resource> &)) &smtk::bridge::multiscale::Resource::create, py::arg("ref"))
    .def("session", &smtk::bridge::multiscale::Resource::session)
    .def("setSession", &smtk::bridge::multiscale::Resource::setSession)
    .def_static("CastTo", [](const std::shared_ptr<smtk::resource::Resource> i) {
        return std::dynamic_pointer_cast<smtk::bridge::multiscale::Resource>(i);
      })
    ;
  return instance;
}

#endif
