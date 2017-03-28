//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_multiscale_operators_PartitionBoundariesOperator_h
#define pybind_smtk_bridge_multiscale_operators_PartitionBoundariesOperator_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/multiscale/operators/PartitionBoundariesOperator.h"

#include "smtk/bridge/multiscale/Operator.h"
#include "smtk/model/Operator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::multiscale::PartitionBoundariesOperator, smtk::bridge::multiscale::Operator > pybind11_init_smtk_bridge_multiscale_PartitionBoundariesOperator(py::module &m)
{
  PySharedPtrClass< smtk::bridge::multiscale::PartitionBoundariesOperator, smtk::bridge::multiscale::Operator > instance(m, "PartitionBoundariesOperator");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::multiscale::PartitionBoundariesOperator const &>())
    .def("deepcopy", (smtk::bridge::multiscale::PartitionBoundariesOperator & (smtk::bridge::multiscale::PartitionBoundariesOperator::*)(::smtk::bridge::multiscale::PartitionBoundariesOperator const &)) &smtk::bridge::multiscale::PartitionBoundariesOperator::operator=)
    .def_static("baseCreate", &smtk::bridge::multiscale::PartitionBoundariesOperator::baseCreate)
    .def("className", &smtk::bridge::multiscale::PartitionBoundariesOperator::className)
    .def("classname", &smtk::bridge::multiscale::PartitionBoundariesOperator::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::multiscale::PartitionBoundariesOperator> (*)()) &smtk::bridge::multiscale::PartitionBoundariesOperator::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::multiscale::PartitionBoundariesOperator> (*)(::std::shared_ptr<smtk::bridge::multiscale::PartitionBoundariesOperator> &)) &smtk::bridge::multiscale::PartitionBoundariesOperator::create, py::arg("ref"))
    .def("name", &smtk::bridge::multiscale::PartitionBoundariesOperator::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::multiscale::PartitionBoundariesOperator> (smtk::bridge::multiscale::PartitionBoundariesOperator::*)() const) &smtk::bridge::multiscale::PartitionBoundariesOperator::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::multiscale::PartitionBoundariesOperator> (smtk::bridge::multiscale::PartitionBoundariesOperator::*)()) &smtk::bridge::multiscale::PartitionBoundariesOperator::shared_from_this)
    .def_readwrite_static("operatorName", &smtk::bridge::multiscale::PartitionBoundariesOperator::operatorName)
    ;
  return instance;
}

#endif
