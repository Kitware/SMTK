//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_discrete_operators_ImportOperator_h
#define pybind_smtk_bridge_discrete_operators_ImportOperator_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/discrete/operators/ImportOperator.h"

#include "smtk/model/Operator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::discrete::ImportOperator, smtk::model::Operator > pybind11_init_smtk_bridge_discrete_ImportOperator(py::module &m)
{
  PySharedPtrClass< smtk::bridge::discrete::ImportOperator, smtk::model::Operator > instance(m, "ImportOperator");
  instance
    .def("classname", &smtk::bridge::discrete::ImportOperator::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::ImportOperator> (*)()) &smtk::bridge::discrete::ImportOperator::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::ImportOperator> (*)(::std::shared_ptr<smtk::bridge::discrete::ImportOperator> &)) &smtk::bridge::discrete::ImportOperator::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::discrete::ImportOperator> (smtk::bridge::discrete::ImportOperator::*)()) &smtk::bridge::discrete::ImportOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::discrete::ImportOperator> (smtk::bridge::discrete::ImportOperator::*)() const) &smtk::bridge::discrete::ImportOperator::shared_from_this)
    .def("name", &smtk::bridge::discrete::ImportOperator::name)
    .def("className", &smtk::bridge::discrete::ImportOperator::className)
    .def_static("baseCreate", &smtk::bridge::discrete::ImportOperator::baseCreate)
    .def("ableToOperate", &smtk::bridge::discrete::ImportOperator::ableToOperate)
    ;
  return instance;
}

#endif
