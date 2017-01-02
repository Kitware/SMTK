//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_multiscale_operators_RevolveOperator_h
#define pybind_smtk_bridge_multiscale_operators_RevolveOperator_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/multiscale/operators/RevolveOperator.h"

#include "smtk/bridge/multiscale/Operator.h"
#include "smtk/model/Operator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::multiscale::RevolveOperator, smtk::bridge::multiscale::Operator > pybind11_init_smtk_bridge_multiscale_RevolveOperator(py::module &m)
{
  PySharedPtrClass< smtk::bridge::multiscale::RevolveOperator, smtk::bridge::multiscale::Operator > instance(m, "RevolveOperator");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::multiscale::RevolveOperator const &>())
    .def("deepcopy", (smtk::bridge::multiscale::RevolveOperator & (smtk::bridge::multiscale::RevolveOperator::*)(::smtk::bridge::multiscale::RevolveOperator const &)) &smtk::bridge::multiscale::RevolveOperator::operator=)
    .def_static("baseCreate", &smtk::bridge::multiscale::RevolveOperator::baseCreate)
    .def("className", &smtk::bridge::multiscale::RevolveOperator::className)
    .def("classname", &smtk::bridge::multiscale::RevolveOperator::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::multiscale::RevolveOperator> (*)()) &smtk::bridge::multiscale::RevolveOperator::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::multiscale::RevolveOperator> (*)(::std::shared_ptr<smtk::bridge::multiscale::RevolveOperator> &)) &smtk::bridge::multiscale::RevolveOperator::create, py::arg("ref"))
    .def("name", &smtk::bridge::multiscale::RevolveOperator::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::multiscale::RevolveOperator> (smtk::bridge::multiscale::RevolveOperator::*)() const) &smtk::bridge::multiscale::RevolveOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::multiscale::RevolveOperator> (smtk::bridge::multiscale::RevolveOperator::*)()) &smtk::bridge::multiscale::RevolveOperator::shared_from_this)
    .def_readwrite_static("operatorName", &smtk::bridge::multiscale::RevolveOperator::operatorName)
    ;
  return instance;
}

#endif
