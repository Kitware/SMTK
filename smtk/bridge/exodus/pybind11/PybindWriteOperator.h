//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_exodus_WriteOperator_h
#define pybind_smtk_bridge_exodus_WriteOperator_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/exodus/WriteOperator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::exodus::WriteOperator > pybind11_init_smtk_bridge_exodus_WriteOperator(py::module &m, PySharedPtrClass< smtk::bridge::exodus::Operator, smtk::model::Operator >& parent)
{
  PySharedPtrClass< smtk::bridge::exodus::WriteOperator > instance(m, "WriteOperator", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::exodus::WriteOperator const &>())
    .def("deepcopy", (smtk::bridge::exodus::WriteOperator & (smtk::bridge::exodus::WriteOperator::*)(::smtk::bridge::exodus::WriteOperator const &)) &smtk::bridge::exodus::WriteOperator::operator=)
    .def_static("baseCreate", &smtk::bridge::exodus::WriteOperator::baseCreate)
    .def("className", &smtk::bridge::exodus::WriteOperator::className)
    .def("classname", &smtk::bridge::exodus::WriteOperator::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::exodus::WriteOperator> (*)()) &smtk::bridge::exodus::WriteOperator::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::exodus::WriteOperator> (*)(::std::shared_ptr<smtk::bridge::exodus::WriteOperator> &)) &smtk::bridge::exodus::WriteOperator::create, py::arg("ref"))
    .def("name", &smtk::bridge::exodus::WriteOperator::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::exodus::WriteOperator> (smtk::bridge::exodus::WriteOperator::*)() const) &smtk::bridge::exodus::WriteOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::exodus::WriteOperator> (smtk::bridge::exodus::WriteOperator::*)()) &smtk::bridge::exodus::WriteOperator::shared_from_this)
    ;
  return instance;
}

#endif
