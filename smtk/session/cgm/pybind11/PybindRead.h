//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_cgm_operators_Read_h
#define pybind_smtk_session_cgm_operators_Read_h

#include <pybind11/pybind11.h>

#include "smtk/session/cgm/operators/Read.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::cgm::Read > pybind11_init_smtk_session_cgm_Read(py::module &m, PySharedPtrClass< smtk::session::cgm::Operation, smtk::operation::Operation >& parent)
{
  PySharedPtrClass< smtk::session::cgm::Read > instance(m, "Read", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::session::cgm::Read const &>())
    .def("deepcopy", (smtk::session::cgm::Read & (smtk::session::cgm::Read::*)(::smtk::session::cgm::Read const &)) &smtk::session::cgm::Read::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::Read> (*)()) &smtk::session::cgm::Read::create)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::Read> (*)(::std::shared_ptr<smtk::session::cgm::Read> &)) &smtk::session::cgm::Read::create, py::arg("ref"))
    .def("name", &smtk::session::cgm::Read::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::cgm::Read> (smtk::session::cgm::Read::*)() const) &smtk::session::cgm::Read::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::cgm::Read> (smtk::session::cgm::Read::*)()) &smtk::session::cgm::Read::shared_from_this)
    ;
  return instance;
}

#endif
