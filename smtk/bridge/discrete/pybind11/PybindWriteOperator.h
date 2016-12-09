//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_discrete_operators_WriteOperator_h
#define pybind_smtk_bridge_discrete_operators_WriteOperator_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/discrete/operators/WriteOperator.h"

#include "smtk/model/Operator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::discrete::WriteOperator, smtk::model::Operator > pybind11_init_smtk_bridge_discrete_WriteOperator(py::module &m)
{
  PySharedPtrClass< smtk::bridge::discrete::WriteOperator, smtk::model::Operator > instance(m, "WriteOperator");
  instance
    .def("classname", &smtk::bridge::discrete::WriteOperator::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::WriteOperator> (*)()) &smtk::bridge::discrete::WriteOperator::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::WriteOperator> (*)(::std::shared_ptr<smtk::bridge::discrete::WriteOperator> &)) &smtk::bridge::discrete::WriteOperator::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::discrete::WriteOperator> (smtk::bridge::discrete::WriteOperator::*)()) &smtk::bridge::discrete::WriteOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::discrete::WriteOperator> (smtk::bridge::discrete::WriteOperator::*)() const) &smtk::bridge::discrete::WriteOperator::shared_from_this)
    .def("name", &smtk::bridge::discrete::WriteOperator::name)
    .def("className", &smtk::bridge::discrete::WriteOperator::className)
    .def_static("baseCreate", &smtk::bridge::discrete::WriteOperator::baseCreate)
    .def("ableToOperate", &smtk::bridge::discrete::WriteOperator::ableToOperate)
    ;
  return instance;
}

#endif
