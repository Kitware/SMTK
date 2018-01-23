//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_operation_ImportPythonOperator_h
#define pybind_smtk_operation_ImportPythonOperator_h

#include <pybind11/pybind11.h>

#include "smtk/operation/ImportPythonOperator.h"

#include "smtk/operation/XMLOperator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::operation::ImportPythonOperator, smtk::operation::XMLOperator > pybind11_init_smtk_operation_ImportPythonOperator(py::module &m)
{
  PySharedPtrClass< smtk::operation::ImportPythonOperator, smtk::operation::XMLOperator > instance(m, "ImportPythonOperator");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::operation::ImportPythonOperator const &>())
    .def("deepcopy", (smtk::operation::ImportPythonOperator & (smtk::operation::ImportPythonOperator::*)(::smtk::operation::ImportPythonOperator const &)) &smtk::operation::ImportPythonOperator::operator=)
    .def("className", &smtk::operation::ImportPythonOperator::className)
    .def_static("create", (std::shared_ptr<smtk::operation::ImportPythonOperator> (*)()) &smtk::operation::ImportPythonOperator::create)
    .def_static("create", (std::shared_ptr<smtk::operation::ImportPythonOperator> (*)(::std::shared_ptr<smtk::operation::ImportPythonOperator> &)) &smtk::operation::ImportPythonOperator::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::operation::ImportPythonOperator> (smtk::operation::ImportPythonOperator::*)() const) &smtk::operation::ImportPythonOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::operation::ImportPythonOperator> (smtk::operation::ImportPythonOperator::*)()) &smtk::operation::ImportPythonOperator::shared_from_this)
    ;
  return instance;
}

#endif
