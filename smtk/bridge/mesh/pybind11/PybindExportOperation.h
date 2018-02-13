//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_mesh_operators_ExportOperation_h
#define pybind_smtk_bridge_mesh_operators_ExportOperation_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/mesh/operators/ExportOperation.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::mesh::ExportOperation, smtk::operation::XMLOperation > pybind11_init_smtk_bridge_mesh_ExportOperation(py::module &m)
{
  PySharedPtrClass< smtk::bridge::mesh::ExportOperation, smtk::operation::XMLOperation > instance(m, "ExportOperation");
  instance
    .def(py::init<::smtk::bridge::mesh::ExportOperation const &>())
    .def(py::init<>())
    .def("deepcopy", (smtk::bridge::mesh::ExportOperation & (smtk::bridge::mesh::ExportOperation::*)(::smtk::bridge::mesh::ExportOperation const &)) &smtk::bridge::mesh::ExportOperation::operator=)
    .def("classname", &smtk::bridge::mesh::ExportOperation::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::mesh::ExportOperation> (*)()) &smtk::bridge::mesh::ExportOperation::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::mesh::ExportOperation> (*)(::std::shared_ptr<smtk::bridge::mesh::ExportOperation> &)) &smtk::bridge::mesh::ExportOperation::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::mesh::ExportOperation> (smtk::bridge::mesh::ExportOperation::*)()) &smtk::bridge::mesh::ExportOperation::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::mesh::ExportOperation> (smtk::bridge::mesh::ExportOperation::*)() const) &smtk::bridge::mesh::ExportOperation::shared_from_this)
    ;
  return instance;
}

#endif
