//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_mesh_operators_WriteOperator_h
#define pybind_smtk_bridge_mesh_operators_WriteOperator_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/mesh/operators/WriteOperator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::mesh::WriteOperator > pybind11_init_smtk_bridge_mesh_WriteOperator(py::module &m, PySharedPtrClass< smtk::bridge::mesh::Operator, smtk::model::Operator >& parent)
{
  PySharedPtrClass< smtk::bridge::mesh::WriteOperator > instance(m, "WriteOperator", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::mesh::WriteOperator const &>())
    .def("deepcopy", (smtk::bridge::mesh::WriteOperator & (smtk::bridge::mesh::WriteOperator::*)(::smtk::bridge::mesh::WriteOperator const &)) &smtk::bridge::mesh::WriteOperator::operator=)
    .def_static("baseCreate", &smtk::bridge::mesh::WriteOperator::baseCreate)
    .def("className", &smtk::bridge::mesh::WriteOperator::className)
    .def("classname", &smtk::bridge::mesh::WriteOperator::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::mesh::WriteOperator> (*)()) &smtk::bridge::mesh::WriteOperator::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::mesh::WriteOperator> (*)(::std::shared_ptr<smtk::bridge::mesh::WriteOperator> &)) &smtk::bridge::mesh::WriteOperator::create, py::arg("ref"))
    .def("name", &smtk::bridge::mesh::WriteOperator::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::mesh::WriteOperator> (smtk::bridge::mesh::WriteOperator::*)() const) &smtk::bridge::mesh::WriteOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::mesh::WriteOperator> (smtk::bridge::mesh::WriteOperator::*)()) &smtk::bridge::mesh::WriteOperator::shared_from_this)
    ;
  return instance;
}

#endif
