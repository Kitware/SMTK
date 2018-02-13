//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_discrete_operators_GrowOperation_h
#define pybind_smtk_bridge_discrete_operators_GrowOperation_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/discrete/operators/GrowOperation.h"


namespace py = pybind11;

PySharedPtrClass< smtk::bridge::discrete::GrowOperation, smtk::operation::Operation > pybind11_init_smtk_bridge_discrete_GrowOperation(py::module &m)
{
  PySharedPtrClass< smtk::bridge::discrete::GrowOperation, smtk::operation::Operation > instance(m, "GrowOperation");
  instance
    .def("classname", &smtk::bridge::discrete::GrowOperation::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::GrowOperation> (*)()) &smtk::bridge::discrete::GrowOperation::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::GrowOperation> (*)(::std::shared_ptr<smtk::bridge::discrete::GrowOperation> &)) &smtk::bridge::discrete::GrowOperator::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::discrete::GrowOperation> (smtk::bridge::discrete::GrowOperation::*)()) &smtk::bridge::discrete::GrowOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::discrete::GrowOperation> (smtk::bridge::discrete::GrowOperation::*)() const) &smtk::bridge::discrete::GrowOperator::shared_from_this)
    .def("name", &smtk::bridge::discrete::GrowOperation::name)
    .def("className", &smtk::bridge::discrete::GrowOperation::className)
    .def("ableToOperate", &smtk::bridge::discrete::GrowOperation::ableToOperate)
    ;
  return instance;
}

#endif
