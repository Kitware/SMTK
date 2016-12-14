//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_mesh_Session_h
#define pybind_smtk_bridge_mesh_Session_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/mesh/Session.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::mesh::Session, smtk::model::Session > pybind11_init_smtk_bridge_mesh_Session(py::module &m)
{
  PySharedPtrClass< smtk::bridge::mesh::Session, smtk::model::Session > instance(m, "Session");
  instance
    .def(py::init<::smtk::bridge::mesh::Session const &>())
    .def("deepcopy", (smtk::bridge::mesh::Session & (smtk::bridge::mesh::Session::*)(::smtk::bridge::mesh::Session const &)) &smtk::bridge::mesh::Session::operator=)
    .def("addTopology", &smtk::bridge::mesh::Session::addTopology, py::arg("t"))
    .def("className", &smtk::bridge::mesh::Session::className)
    .def_static("create", (std::shared_ptr<smtk::bridge::mesh::Session> (*)()) &smtk::bridge::mesh::Session::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::mesh::Session> (*)(::std::shared_ptr<smtk::bridge::mesh::Session> &)) &smtk::bridge::mesh::Session::create, py::arg("ref"))
    .def("topology", &smtk::bridge::mesh::Session::topology, py::arg("model"))
    ;
  return instance;
}

#endif
