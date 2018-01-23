//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_mesh_operators_ExportOperator_h
#define pybind_smtk_bridge_mesh_operators_ExportOperator_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/mesh/operators/ExportOperator.h"

#include "smtk/operation/XMLOperator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::mesh::ExportOperator, smtk::operation::XMLOperator > pybind11_init_smtk_bridge_mesh_ExportOperator(py::module &m)
{
  PySharedPtrClass< smtk::bridge::mesh::ExportOperator, smtk::operation::XMLOperator > instance(m, "ExportOperator");
  instance
    .def(py::init<::smtk::bridge::mesh::ExportOperator const &>())
    .def(py::init<>())
    .def("deepcopy", (smtk::bridge::mesh::ExportOperator & (smtk::bridge::mesh::ExportOperator::*)(::smtk::bridge::mesh::ExportOperator const &)) &smtk::bridge::mesh::ExportOperator::operator=)
    .def("classname", &smtk::bridge::mesh::ExportOperator::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::mesh::ExportOperator> (*)()) &smtk::bridge::mesh::ExportOperator::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::mesh::ExportOperator> (*)(::std::shared_ptr<smtk::bridge::mesh::ExportOperator> &)) &smtk::bridge::mesh::ExportOperator::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::mesh::ExportOperator> (smtk::bridge::mesh::ExportOperator::*)()) &smtk::bridge::mesh::ExportOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::mesh::ExportOperator> (smtk::bridge::mesh::ExportOperator::*)() const) &smtk::bridge::mesh::ExportOperator::shared_from_this)
    ;
  return instance;
}

#endif
