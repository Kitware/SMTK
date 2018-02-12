//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_discrete_operators_MergeOperation_h
#define pybind_smtk_bridge_discrete_operators_MergeOperation_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/discrete/operators/MergeOperation.h"


namespace py = pybind11;

PySharedPtrClass< smtk::bridge::discrete::MergeOperation, smtk::operation::Operation > pybind11_init_smtk_bridge_discrete_MergeOperation(py::module &m)
{
  PySharedPtrClass< smtk::bridge::discrete::MergeOperation, smtk::operation::Operation > instance(m, "MergeOperation");
  instance
    .def("classname", &smtk::bridge::discrete::MergeOperation::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::MergeOperation> (*)()) &smtk::bridge::discrete::MergeOperation::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::MergeOperation> (*)(::std::shared_ptr<smtk::bridge::discrete::MergeOperation> &)) &smtk::bridge::discrete::MergeOperator::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::discrete::MergeOperation> (smtk::bridge::discrete::MergeOperation::*)()) &smtk::bridge::discrete::MergeOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::discrete::MergeOperation> (smtk::bridge::discrete::MergeOperation::*)() const) &smtk::bridge::discrete::MergeOperator::shared_from_this)
    .def("name", &smtk::bridge::discrete::MergeOperation::name)
    .def("className", &smtk::bridge::discrete::MergeOperation::className)
    .def("ableToOperate", &smtk::bridge::discrete::MergeOperation::ableToOperate)
    ;
  return instance;
}

#endif
