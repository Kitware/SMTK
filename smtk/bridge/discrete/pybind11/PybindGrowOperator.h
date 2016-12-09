//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_discrete_operators_GrowOperator_h
#define pybind_smtk_bridge_discrete_operators_GrowOperator_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/discrete/operators/GrowOperator.h"

#include "smtk/model/Operator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::discrete::GrowOperator, smtk::model::Operator > pybind11_init_smtk_bridge_discrete_GrowOperator(py::module &m)
{
  PySharedPtrClass< smtk::bridge::discrete::GrowOperator, smtk::model::Operator > instance(m, "GrowOperator");
  instance
    .def("classname", &smtk::bridge::discrete::GrowOperator::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::GrowOperator> (*)()) &smtk::bridge::discrete::GrowOperator::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::GrowOperator> (*)(::std::shared_ptr<smtk::bridge::discrete::GrowOperator> &)) &smtk::bridge::discrete::GrowOperator::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::discrete::GrowOperator> (smtk::bridge::discrete::GrowOperator::*)()) &smtk::bridge::discrete::GrowOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::discrete::GrowOperator> (smtk::bridge::discrete::GrowOperator::*)() const) &smtk::bridge::discrete::GrowOperator::shared_from_this)
    .def("name", &smtk::bridge::discrete::GrowOperator::name)
    .def("className", &smtk::bridge::discrete::GrowOperator::className)
    .def_static("baseCreate", &smtk::bridge::discrete::GrowOperator::baseCreate)
    .def("ableToOperate", &smtk::bridge::discrete::GrowOperator::ableToOperate)
    ;
  return instance;
}

#endif
