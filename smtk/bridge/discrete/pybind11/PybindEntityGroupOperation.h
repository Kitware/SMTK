//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_discrete_operators_EntityGroupOperation_h
#define pybind_smtk_bridge_discrete_operators_EntityGroupOperation_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/discrete/operators/EntityGroupOperation.h"


namespace py = pybind11;

PySharedPtrClass< smtk::bridge::discrete::EntityGroupOperation, smtk::operation::Operation > pybind11_init_smtk_bridge_discrete_EntityGroupOperation(py::module &m)
{
  PySharedPtrClass< smtk::bridge::discrete::EntityGroupOperation, smtk::operation::Operation > instance(m, "EntityGroupOperation");
  instance
    .def("classname", &smtk::bridge::discrete::EntityGroupOperation::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::EntityGroupOperation> (*)()) &smtk::bridge::discrete::EntityGroupOperation::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::EntityGroupOperation> (*)(::std::shared_ptr<smtk::bridge::discrete::EntityGroupOperation> &)) &smtk::bridge::discrete::EntityGroupOperator::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::discrete::EntityGroupOperation> (smtk::bridge::discrete::EntityGroupOperation::*)()) &smtk::bridge::discrete::EntityGroupOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::discrete::EntityGroupOperation> (smtk::bridge::discrete::EntityGroupOperation::*)() const) &smtk::bridge::discrete::EntityGroupOperator::shared_from_this)
    .def("name", &smtk::bridge::discrete::EntityGroupOperation::name)
    .def("className", &smtk::bridge::discrete::EntityGroupOperation::className)
    .def("ableToOperate", &smtk::bridge::discrete::EntityGroupOperation::ableToOperate)
    ;
  return instance;
}

#endif
