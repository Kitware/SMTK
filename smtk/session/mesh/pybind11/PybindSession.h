//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_mesh_Session_h
#define pybind_smtk_session_mesh_Session_h

#include <pybind11/pybind11.h>

#include "smtk/session/mesh/Session.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::session::mesh::Session, smtk::model::Session > pybind11_init_smtk_session_mesh_Session(py::module &m)
{
  PySharedPtrClass< smtk::session::mesh::Session, smtk::model::Session > instance(m, "Session");
  instance
    .def(py::init<::smtk::session::mesh::Session const &>())
    .def("deepcopy", (smtk::session::mesh::Session & (smtk::session::mesh::Session::*)(::smtk::session::mesh::Session const &)) &smtk::session::mesh::Session::operator=)
    .def("addTopology", (void (smtk::session::mesh::Session::*)(smtk::session::mesh::Topology)) &smtk::session::mesh::Session::addTopology, py::arg("t"))
    .def("addTopology", (void (smtk::session::mesh::Session::*)(const std::shared_ptr<smtk::session::mesh::Resource>&, smtk::session::mesh::Topology)) &smtk::session::mesh::Session::addTopology, py::arg("modelResource"), py::arg("t"))
    .def_static("create", (std::shared_ptr<smtk::session::mesh::Session> (*)()) &smtk::session::mesh::Session::create)
    .def_static("create", (std::shared_ptr<smtk::session::mesh::Session> (*)(::std::shared_ptr<smtk::session::mesh::Session> &)) &smtk::session::mesh::Session::create, py::arg("ref"))
    .def("topology", (smtk::session::mesh::Topology* (smtk::session::mesh::Session::*)(const std::shared_ptr<::smtk::session::mesh::Resource>&)) &smtk::session::mesh::Session::topology, py::return_value_policy::reference)
    .def("topology", (smtk::session::mesh::Topology* (smtk::session::mesh::Session::*)(const std::shared_ptr<const ::smtk::session::mesh::Resource>&)) &smtk::session::mesh::Session::topology, py::return_value_policy::reference)
    ;
  return instance;
}

#endif
