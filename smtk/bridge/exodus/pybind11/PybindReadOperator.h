//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_exodus_ReadOperator_h
#define pybind_smtk_bridge_exodus_ReadOperator_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/exodus/ReadOperator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::exodus::ReadOperator > pybind11_init_smtk_bridge_exodus_ReadOperator(py::module &m, PySharedPtrClass< smtk::bridge::exodus::Operator, smtk::model::Operator >& parent)
{
  PySharedPtrClass< smtk::bridge::exodus::ReadOperator > instance(m, "ReadOperator", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::exodus::ReadOperator const &>())
    .def("deepcopy", (smtk::bridge::exodus::ReadOperator & (smtk::bridge::exodus::ReadOperator::*)(::smtk::bridge::exodus::ReadOperator const &)) &smtk::bridge::exodus::ReadOperator::operator=)
    .def_static("baseCreate", &smtk::bridge::exodus::ReadOperator::baseCreate)
    .def("className", &smtk::bridge::exodus::ReadOperator::className)
    .def("classname", &smtk::bridge::exodus::ReadOperator::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::exodus::ReadOperator> (*)()) &smtk::bridge::exodus::ReadOperator::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::exodus::ReadOperator> (*)(::std::shared_ptr<smtk::bridge::exodus::ReadOperator> &)) &smtk::bridge::exodus::ReadOperator::create, py::arg("ref"))
    .def("name", &smtk::bridge::exodus::ReadOperator::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::exodus::ReadOperator> (smtk::bridge::exodus::ReadOperator::*)() const) &smtk::bridge::exodus::ReadOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::exodus::ReadOperator> (smtk::bridge::exodus::ReadOperator::*)()) &smtk::bridge::exodus::ReadOperator::shared_from_this)
    ;
  return instance;
}

#endif
