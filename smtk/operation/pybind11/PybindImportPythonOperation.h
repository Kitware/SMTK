//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_operation_ImportPythonOperation_h
#define pybind_smtk_operation_ImportPythonOperation_h

#include <pybind11/pybind11.h>

#include "smtk/operation/ImportPythonOperation.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

PySharedPtrClass< smtk::operation::ImportPythonOperation, smtk::operation::XMLOperation > pybind11_init_smtk_operation_ImportPythonOperation(py::module &m)
{
  PySharedPtrClass< smtk::operation::ImportPythonOperation, smtk::operation::XMLOperation > instance(m, "ImportPythonOperation");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::operation::ImportPythonOperation const &>())
    .def("deepcopy", (smtk::operation::ImportPythonOperation & (smtk::operation::ImportPythonOperation::*)(::smtk::operation::ImportPythonOperation const &)) &smtk::operation::ImportPythonOperation::operator=)
    .def_static("create", (std::shared_ptr<smtk::operation::ImportPythonOperation> (*)()) &smtk::operation::ImportPythonOperation::create)
    .def_static("create", (std::shared_ptr<smtk::operation::ImportPythonOperation> (*)(::std::shared_ptr<smtk::operation::ImportPythonOperation> &)) &smtk::operation::ImportPythonOperation::create, py::arg("ref"))
    .def_static("importOperationsFromModule", &smtk::operation::ImportPythonOperation::importOperationsFromModule, py::arg("module"), py::arg("operationManager"))
    .def_static("importOperation", &smtk::operation::ImportPythonOperation::importOperation, py::arg("operationManager"), py::arg("module"), py::arg("operationName"))
    .def("shared_from_this", (std::shared_ptr<const smtk::operation::ImportPythonOperation> (smtk::operation::ImportPythonOperation::*)() const) &smtk::operation::ImportPythonOperation::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::operation::ImportPythonOperation> (smtk::operation::ImportPythonOperation::*)()) &smtk::operation::ImportPythonOperation::shared_from_this)
    ;
  return instance;
}

#endif
