//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_discrete_operators_MergeOperator_h
#define pybind_smtk_bridge_discrete_operators_MergeOperator_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/discrete/operators/MergeOperator.h"

#include "smtk/model/Operator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::discrete::MergeOperator, smtk::model::Operator > pybind11_init_smtk_bridge_discrete_MergeOperator(py::module &m)
{
  PySharedPtrClass< smtk::bridge::discrete::MergeOperator, smtk::model::Operator > instance(m, "MergeOperator");
  instance
    .def("classname", &smtk::bridge::discrete::MergeOperator::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::MergeOperator> (*)()) &smtk::bridge::discrete::MergeOperator::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::MergeOperator> (*)(::std::shared_ptr<smtk::bridge::discrete::MergeOperator> &)) &smtk::bridge::discrete::MergeOperator::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::discrete::MergeOperator> (smtk::bridge::discrete::MergeOperator::*)()) &smtk::bridge::discrete::MergeOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::discrete::MergeOperator> (smtk::bridge::discrete::MergeOperator::*)() const) &smtk::bridge::discrete::MergeOperator::shared_from_this)
    .def("name", &smtk::bridge::discrete::MergeOperator::name)
    .def("className", &smtk::bridge::discrete::MergeOperator::className)
    .def_static("baseCreate", &smtk::bridge::discrete::MergeOperator::baseCreate)
    .def("ableToOperate", &smtk::bridge::discrete::MergeOperator::ableToOperate)
    ;
  return instance;
}

#endif
