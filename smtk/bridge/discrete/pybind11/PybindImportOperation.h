//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_discrete_operators_ImportOperation_h
#define pybind_smtk_bridge_discrete_operators_ImportOperation_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/discrete/operators/ImportOperation.h"


namespace py = pybind11;

PySharedPtrClass< smtk::bridge::discrete::ImportOperation, smtk::operation::Operation > pybind11_init_smtk_bridge_discrete_ImportOperation(py::module &m)
{
  PySharedPtrClass< smtk::bridge::discrete::ImportOperation, smtk::operation::Operation > instance(m, "ImportOperation");
  instance
    .def("classname", &smtk::bridge::discrete::ImportOperation::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::ImportOperation> (*)()) &smtk::bridge::discrete::ImportOperation::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::ImportOperation> (*)(::std::shared_ptr<smtk::bridge::discrete::ImportOperation> &)) &smtk::bridge::discrete::ImportOperator::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::discrete::ImportOperation> (smtk::bridge::discrete::ImportOperation::*)()) &smtk::bridge::discrete::ImportOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::discrete::ImportOperation> (smtk::bridge::discrete::ImportOperation::*)() const) &smtk::bridge::discrete::ImportOperator::shared_from_this)
    .def("name", &smtk::bridge::discrete::ImportOperation::name)
    .def("className", &smtk::bridge::discrete::ImportOperation::className)
    .def("ableToOperate", &smtk::bridge::discrete::ImportOperation::ableToOperate)
    ;
  return instance;
}

#endif
