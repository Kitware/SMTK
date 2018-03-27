//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_discrete_operators_CreateEdgesOperation_h
#define pybind_smtk_bridge_discrete_operators_CreateEdgesOperation_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/discrete/operators/CreateEdgesOperation.h"


namespace py = pybind11;

PySharedPtrClass< smtk::bridge::discrete::CreateEdgesOperation, smtk::operation::Operation > pybind11_init_smtk_bridge_discrete_CreateEdgesOperation(py::module &m)
{
  PySharedPtrClass< smtk::bridge::discrete::CreateEdgesOperation, smtk::operation::Operation > instance(m, "CreateEdgesOperation");
  instance
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::CreateEdgesOperation> (*)()) &smtk::bridge::discrete::CreateEdgesOperation::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::CreateEdgesOperation> (*)(::std::shared_ptr<smtk::bridge::discrete::CreateEdgesOperation> &)) &smtk::bridge::discrete::CreateEdgesOperator::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::discrete::CreateEdgesOperation> (smtk::bridge::discrete::CreateEdgesOperation::*)()) &smtk::bridge::discrete::CreateEdgesOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::discrete::CreateEdgesOperation> (smtk::bridge::discrete::CreateEdgesOperation::*)() const) &smtk::bridge::discrete::CreateEdgesOperator::shared_from_this)
    .def("name", &smtk::bridge::discrete::CreateEdgesOperation::name)
    .def("ableToOperate", &smtk::bridge::discrete::CreateEdgesOperation::ableToOperate)
    ;
  return instance;
}

#endif
