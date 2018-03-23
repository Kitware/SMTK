//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_discrete_operators_WriteOperation_h
#define pybind_smtk_bridge_discrete_operators_WriteOperation_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/discrete/operators/WriteOperation.h"


namespace py = pybind11;

PySharedPtrClass< smtk::bridge::discrete::WriteOperation, smtk::operation::Operation > pybind11_init_smtk_bridge_discrete_WriteOperation(py::module &m)
{
  PySharedPtrClass< smtk::bridge::discrete::WriteOperation, smtk::operation::Operation > instance(m, "WriteOperation");
  instance
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::WriteOperation> (*)()) &smtk::bridge::discrete::WriteOperation::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::WriteOperation> (*)(::std::shared_ptr<smtk::bridge::discrete::WriteOperation> &)) &smtk::bridge::discrete::WriteOperator::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::discrete::WriteOperation> (smtk::bridge::discrete::WriteOperation::*)()) &smtk::bridge::discrete::WriteOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::discrete::WriteOperation> (smtk::bridge::discrete::WriteOperation::*)() const) &smtk::bridge::discrete::WriteOperator::shared_from_this)
    .def("name", &smtk::bridge::discrete::WriteOperation::name)
    .def("ableToOperate", &smtk::bridge::discrete::WriteOperation::ableToOperate)
    ;
  return instance;
}

#endif
