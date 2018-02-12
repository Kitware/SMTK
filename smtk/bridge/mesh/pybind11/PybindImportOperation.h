//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_mesh_operators_ImportOperation_h
#define pybind_smtk_bridge_mesh_operators_ImportOperation_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/mesh/operators/ImportOperation.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::mesh::ImportOperation, smtk::operation::XMLOperation > pybind11_init_smtk_bridge_mesh_ImportOperation(py::module &m)
{
  PySharedPtrClass< smtk::bridge::mesh::ImportOperation, smtk::operation::XMLOperation > instance(m, "ImportOperation");
  instance
    .def(py::init<::smtk::bridge::mesh::ImportOperation const &>())
    .def(py::init<>())
    .def("deepcopy", (smtk::bridge::mesh::ImportOperation & (smtk::bridge::mesh::ImportOperation::*)(::smtk::bridge::mesh::ImportOperation const &)) &smtk::bridge::mesh::ImportOperation::operator=)
    .def("classname", &smtk::bridge::mesh::ImportOperation::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::mesh::ImportOperation> (*)()) &smtk::bridge::mesh::ImportOperation::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::mesh::ImportOperation> (*)(::std::shared_ptr<smtk::bridge::mesh::ImportOperation> &)) &smtk::bridge::mesh::ImportOperation::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::mesh::ImportOperation> (smtk::bridge::mesh::ImportOperation::*)()) &smtk::bridge::mesh::ImportOperation::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::mesh::ImportOperation> (smtk::bridge::mesh::ImportOperation::*)() const) &smtk::bridge::mesh::ImportOperation::shared_from_this)
    ;
  return instance;
}

#endif
