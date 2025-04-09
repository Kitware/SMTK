//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_mesh_operators_Write_h
#define pybind_smtk_session_mesh_operators_Write_h

#include <pybind11/pybind11.h>

#include "smtk/session/mesh/operators/Write.h"

#include "smtk/operation/XMLOperation.h"

#include "smtk/common/Managers.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::session::mesh::Write, smtk::operation::XMLOperation > pybind11_init_smtk_session_mesh_Write(py::module &m)
{
  PySharedPtrClass< smtk::session::mesh::Write, smtk::operation::XMLOperation > instance(m, "Write");
  instance
    .def(py::init<>())
    .def_static("create", (std::shared_ptr<smtk::session::mesh::Write> (*)()) &smtk::session::mesh::Write::create)
    .def_static("create", (std::shared_ptr<smtk::session::mesh::Write> (*)(::std::shared_ptr<smtk::session::mesh::Write> &)) &smtk::session::mesh::Write::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::session::mesh::Write> (smtk::session::mesh::Write::*)()) &smtk::session::mesh::Write::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::mesh::Write> (smtk::session::mesh::Write::*)() const) &smtk::session::mesh::Write::shared_from_this)
    ;

  m.def("write", (bool (*)(const smtk::resource::ResourcePtr&, const std::shared_ptr<smtk::common::Managers>&)) &smtk::session::mesh::write, "", py::arg("resource"), py::arg("managers") = nullptr);

  return instance;
}

#endif
