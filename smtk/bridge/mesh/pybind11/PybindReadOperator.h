//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_mesh_operators_ReadOperator_h
#define pybind_smtk_bridge_mesh_operators_ReadOperator_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/mesh/operators/ReadOperator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::mesh::ReadOperator > pybind11_init_smtk_bridge_mesh_ReadOperator(py::module &m, PySharedPtrClass< smtk::bridge::mesh::Operator, smtk::model::Operator >& parent)
{
  PySharedPtrClass< smtk::bridge::mesh::ReadOperator > instance(m, "ReadOperator", parent);
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::mesh::ReadOperator const &>())
    .def("deepcopy", (smtk::bridge::mesh::ReadOperator & (smtk::bridge::mesh::ReadOperator::*)(::smtk::bridge::mesh::ReadOperator const &)) &smtk::bridge::mesh::ReadOperator::operator=)
    .def_static("baseCreate", &smtk::bridge::mesh::ReadOperator::baseCreate)
    .def("className", &smtk::bridge::mesh::ReadOperator::className)
    .def("classname", &smtk::bridge::mesh::ReadOperator::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::mesh::ReadOperator> (*)()) &smtk::bridge::mesh::ReadOperator::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::mesh::ReadOperator> (*)(::std::shared_ptr<smtk::bridge::mesh::ReadOperator> &)) &smtk::bridge::mesh::ReadOperator::create, py::arg("ref"))
    .def("name", &smtk::bridge::mesh::ReadOperator::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::mesh::ReadOperator> (smtk::bridge::mesh::ReadOperator::*)() const) &smtk::bridge::mesh::ReadOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::mesh::ReadOperator> (smtk::bridge::mesh::ReadOperator::*)()) &smtk::bridge::mesh::ReadOperator::shared_from_this)
    ;
  return instance;
}

#endif
