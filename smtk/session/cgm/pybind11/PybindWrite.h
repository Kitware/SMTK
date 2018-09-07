//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_cgm_operators_Write_h
#define pybind_smtk_session_cgm_operators_Write_h

#include <pybind11/pybind11.h>

#include "smtk/session/cgm/operators/Write.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::cgm::Write > pybind11_init_smtk_session_cgm_Write(py::module &m, PySharedPtrClass< smtk::session::cgm::Operation, smtk::operation::Operation >& parent)
{
  PySharedPtrClass< smtk::session::cgm::Write > instance(m, "Write", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::session::cgm::Write const &>())
    .def("deepcopy", (smtk::session::cgm::Write & (smtk::session::cgm::Write::*)(::smtk::session::cgm::Write const &)) &smtk::session::cgm::Write::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::Write> (*)()) &smtk::session::cgm::Write::create)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::Write> (*)(::std::shared_ptr<smtk::session::cgm::Write> &)) &smtk::session::cgm::Write::create, py::arg("ref"))
    .def("name", &smtk::session::cgm::Write::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::cgm::Write> (smtk::session::cgm::Write::*)() const) &smtk::session::cgm::Write::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::cgm::Write> (smtk::session::cgm::Write::*)()) &smtk::session::cgm::Write::shared_from_this)
    ;
  return instance;
}

#endif
