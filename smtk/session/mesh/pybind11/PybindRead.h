//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_mesh_operators_Read_h
#define pybind_smtk_session_mesh_operators_Read_h

#include <pybind11/pybind11.h>

#include "smtk/session/mesh/operators/Read.h"

#include "smtk/operation/XMLOperation.h"

#include "smtk/common/Managers.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::session::mesh::Read, smtk::operation::XMLOperation > pybind11_init_smtk_session_mesh_Read(py::module &m)
{
  PySharedPtrClass< smtk::session::mesh::Read, smtk::operation::XMLOperation > instance(m, "Read");
  instance
    .def(py::init<>())
    .def_static("create", (std::shared_ptr<smtk::session::mesh::Read> (*)()) &smtk::session::mesh::Read::create)
    .def_static("create", (std::shared_ptr<smtk::session::mesh::Read> (*)(::std::shared_ptr<smtk::session::mesh::Read> &)) &smtk::session::mesh::Read::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::session::mesh::Read> (smtk::session::mesh::Read::*)()) &smtk::session::mesh::Read::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::mesh::Read> (smtk::session::mesh::Read::*)() const) &smtk::session::mesh::Read::shared_from_this)
    ;

  m.def("read", (smtk::resource::ResourcePtr (*)(::std::string const &, const std::shared_ptr<smtk::common::Managers>&)) &smtk::session::mesh::read, "", py::arg("filePath"), py::arg("managers") = nullptr);

  return instance;
}

#endif
