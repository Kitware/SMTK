//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_discrete_operators_SplitFaceOperator_h
#define pybind_smtk_bridge_discrete_operators_SplitFaceOperator_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/discrete/operators/SplitFaceOperator.h"

#include "smtk/model/Operator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::discrete::SplitFaceOperator, smtk::model::Operator > pybind11_init_smtk_bridge_discrete_SplitFaceOperator(py::module &m)
{
  PySharedPtrClass< smtk::bridge::discrete::SplitFaceOperator, smtk::model::Operator > instance(m, "SplitFaceOperator");
  instance
    .def("classname", &smtk::bridge::discrete::SplitFaceOperator::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::SplitFaceOperator> (*)()) &smtk::bridge::discrete::SplitFaceOperator::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::SplitFaceOperator> (*)(::std::shared_ptr<smtk::bridge::discrete::SplitFaceOperator> &)) &smtk::bridge::discrete::SplitFaceOperator::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::discrete::SplitFaceOperator> (smtk::bridge::discrete::SplitFaceOperator::*)()) &smtk::bridge::discrete::SplitFaceOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::discrete::SplitFaceOperator> (smtk::bridge::discrete::SplitFaceOperator::*)() const) &smtk::bridge::discrete::SplitFaceOperator::shared_from_this)
    .def("name", &smtk::bridge::discrete::SplitFaceOperator::name)
    .def("className", &smtk::bridge::discrete::SplitFaceOperator::className)
    .def_static("baseCreate", &smtk::bridge::discrete::SplitFaceOperator::baseCreate)
    .def("ableToOperate", &smtk::bridge::discrete::SplitFaceOperator::ableToOperate)
    ;
  return instance;
}

#endif
