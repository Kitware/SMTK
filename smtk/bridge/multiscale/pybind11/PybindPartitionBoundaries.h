//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_multiscale_operators_PartitionBoundaries_h
#define pybind_smtk_bridge_multiscale_operators_PartitionBoundaries_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/multiscale/operators/PartitionBoundaries.h"

#include "smtk/bridge/multiscale/Operator.h"
#include "smtk/model/Operator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::multiscale::PartitionBoundaries, smtk::bridge::multiscale::Operator > pybind11_init_smtk_bridge_multiscale_PartitionBoundaries(py::module &m)
{
  PySharedPtrClass< smtk::bridge::multiscale::PartitionBoundaries, smtk::bridge::multiscale::Operator > instance(m, "PartitionBoundaries");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::multiscale::PartitionBoundaries const &>())
    .def("deepcopy", (smtk::bridge::multiscale::PartitionBoundaries & (smtk::bridge::multiscale::PartitionBoundaries::*)(::smtk::bridge::multiscale::PartitionBoundaries const &)) &smtk::bridge::multiscale::PartitionBoundaries::operator=)
    .def_static("baseCreate", &smtk::bridge::multiscale::PartitionBoundaries::baseCreate)
    .def("className", &smtk::bridge::multiscale::PartitionBoundaries::className)
    .def("classname", &smtk::bridge::multiscale::PartitionBoundaries::classname)
    .def_static("create", (std::shared_ptr<smtk::bridge::multiscale::PartitionBoundaries> (*)()) &smtk::bridge::multiscale::PartitionBoundaries::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::multiscale::PartitionBoundaries> (*)(::std::shared_ptr<smtk::bridge::multiscale::PartitionBoundaries> &)) &smtk::bridge::multiscale::PartitionBoundaries::create, py::arg("ref"))
    .def("name", &smtk::bridge::multiscale::PartitionBoundaries::name)
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::multiscale::PartitionBoundaries> (smtk::bridge::multiscale::PartitionBoundaries::*)() const) &smtk::bridge::multiscale::PartitionBoundaries::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::multiscale::PartitionBoundaries> (smtk::bridge::multiscale::PartitionBoundaries::*)()) &smtk::bridge::multiscale::PartitionBoundaries::shared_from_this)
    .def_readwrite_static("operatorName", &smtk::bridge::multiscale::PartitionBoundaries::operatorName)
    ;
  return instance;
}

#endif
