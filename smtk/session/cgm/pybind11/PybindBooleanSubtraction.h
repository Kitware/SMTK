//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_cgm_operators_BooleanSubtraction_h
#define pybind_smtk_session_cgm_operators_BooleanSubtraction_h

#include <pybind11/pybind11.h>

#include "smtk/session/cgm/operators/BooleanSubtraction.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::cgm::BooleanSubtraction > pybind11_init_smtk_session_cgm_BooleanSubtraction(py::module &m, PySharedPtrClass< smtk::session::cgm::Operation, smtk::operation::Operation >& parent)
{
  PySharedPtrClass< smtk::session::cgm::BooleanSubtraction > instance(m, "BooleanSubtraction", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::session::cgm::BooleanSubtraction const &>())
    .def("deepcopy", (smtk::session::cgm::BooleanSubtraction & (smtk::session::cgm::BooleanSubtraction::*)(::smtk::session::cgm::BooleanSubtraction const &)) &smtk::session::cgm::BooleanSubtraction::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::BooleanSubtraction> (*)()) &smtk::session::cgm::BooleanSubtraction::create)
    .def_static("create", (std::shared_ptr<smtk::session::cgm::BooleanSubtraction> (*)(::std::shared_ptr<smtk::session::cgm::BooleanSubtraction> &)) &smtk::session::cgm::BooleanSubtraction::create, py::arg("ref"))
    .def("name", &smtk::session::cgm::BooleanSubtraction::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::session::cgm::BooleanSubtraction> (smtk::session::cgm::BooleanSubtraction::*)() const) &smtk::session::cgm::BooleanSubtraction::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::cgm::BooleanSubtraction> (smtk::session::cgm::BooleanSubtraction::*)()) &smtk::session::cgm::BooleanSubtraction::shared_from_this)
    ;
  return instance;
}

#endif
