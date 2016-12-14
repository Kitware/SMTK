//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_mesh_operators_ImportOperator_h
#define pybind_smtk_bridge_mesh_operators_ImportOperator_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/mesh/operators/ImportOperator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::mesh::ImportOperator > pybind11_init_smtk_bridge_mesh_ImportOperator(py::module &m, PySharedPtrClass< smtk::bridge::mesh::Operator, smtk::model::Operator >& parent)
{
  PySharedPtrClass< smtk::bridge::mesh::ImportOperator > instance(m, "ImportOperator", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::mesh::ImportOperator const &>())
    .def("deepcopy", (smtk::bridge::mesh::ImportOperator & (smtk::bridge::mesh::ImportOperator::*)(::smtk::bridge::mesh::ImportOperator const &)) &smtk::bridge::mesh::ImportOperator::operator=)
    .def_static("baseCreate", &smtk::bridge::mesh::ImportOperator::baseCreate)
    .def("className", &smtk::bridge::mesh::ImportOperator::className)
    .def("classname", &smtk::bridge::mesh::ImportOperator::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::mesh::ImportOperator> (*)()) &smtk::bridge::mesh::ImportOperator::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::mesh::ImportOperator> (*)(::std::shared_ptr<smtk::bridge::mesh::ImportOperator> &)) &smtk::bridge::mesh::ImportOperator::create, py::arg("ref"))
    .def("name", &smtk::bridge::mesh::ImportOperator::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::mesh::ImportOperator> (smtk::bridge::mesh::ImportOperator::*)() const) &smtk::bridge::mesh::ImportOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::mesh::ImportOperator> (smtk::bridge::mesh::ImportOperator::*)()) &smtk::bridge::mesh::ImportOperator::shared_from_this)
    ;
  return instance;
}

#endif
