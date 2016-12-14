//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_cgm_operators_Write_h
#define pybind_smtk_bridge_cgm_operators_Write_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/cgm/operators/Write.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::cgm::Write > pybind11_init_smtk_bridge_cgm_Write(py::module &m, PySharedPtrClass< smtk::bridge::cgm::Operator, smtk::model::Operator >& parent)
{
  PySharedPtrClass< smtk::bridge::cgm::Write > instance(m, "Write", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::cgm::Write const &>())
    .def("deepcopy", (smtk::bridge::cgm::Write & (smtk::bridge::cgm::Write::*)(::smtk::bridge::cgm::Write const &)) &smtk::bridge::cgm::Write::operator=)
    .def_static("baseCreate", &smtk::bridge::cgm::Write::baseCreate)
    .def("className", &smtk::bridge::cgm::Write::className)
    .def("classname", &smtk::bridge::cgm::Write::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::Write> (*)()) &smtk::bridge::cgm::Write::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::cgm::Write> (*)(::std::shared_ptr<smtk::bridge::cgm::Write> &)) &smtk::bridge::cgm::Write::create, py::arg("ref"))
    .def("name", &smtk::bridge::cgm::Write::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::cgm::Write> (smtk::bridge::cgm::Write::*)() const) &smtk::bridge::cgm::Write::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::cgm::Write> (smtk::bridge::cgm::Write::*)()) &smtk::bridge::cgm::Write::shared_from_this)
    ;
  return instance;
}

#endif
