//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_discrete_operators_CreateEdgesOperator_h
#define pybind_smtk_bridge_discrete_operators_CreateEdgesOperator_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/discrete/operators/CreateEdgesOperator.h"

#include "smtk/model/Operator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::discrete::CreateEdgesOperator, smtk::model::Operator > pybind11_init_smtk_bridge_discrete_CreateEdgesOperator(py::module &m)
{
  PySharedPtrClass< smtk::bridge::discrete::CreateEdgesOperator, smtk::model::Operator > instance(m, "CreateEdgesOperator");
  instance
    .def("classname", &smtk::bridge::discrete::CreateEdgesOperator::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::CreateEdgesOperator> (*)()) &smtk::bridge::discrete::CreateEdgesOperator::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::discrete::CreateEdgesOperator> (*)(::std::shared_ptr<smtk::bridge::discrete::CreateEdgesOperator> &)) &smtk::bridge::discrete::CreateEdgesOperator::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::discrete::CreateEdgesOperator> (smtk::bridge::discrete::CreateEdgesOperator::*)()) &smtk::bridge::discrete::CreateEdgesOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::discrete::CreateEdgesOperator> (smtk::bridge::discrete::CreateEdgesOperator::*)() const) &smtk::bridge::discrete::CreateEdgesOperator::shared_from_this)
    .def("name", &smtk::bridge::discrete::CreateEdgesOperator::name)
    .def("className", &smtk::bridge::discrete::CreateEdgesOperator::className)
    .def_static("baseCreate", &smtk::bridge::discrete::CreateEdgesOperator::baseCreate)
    .def("ableToOperate", &smtk::bridge::discrete::CreateEdgesOperator::ableToOperate)
    ;
  return instance;
}

#endif
