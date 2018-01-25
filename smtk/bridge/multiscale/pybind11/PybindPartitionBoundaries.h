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

#include "smtk/operation/XMLOperator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::bridge::multiscale::PartitionBoundaries, smtk::operation::XMLOperator > pybind11_init_smtk_bridge_multiscale_PartitionBoundaries(py::module &m)
{
  PySharedPtrClass< smtk::bridge::multiscale::PartitionBoundaries, smtk::operation::XMLOperator > instance(m, "PartitionBoundaries");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::multiscale::PartitionBoundaries const &>())
    .def("deepcopy", (smtk::bridge::multiscale::PartitionBoundaries & (smtk::bridge::multiscale::PartitionBoundaries::*)(::smtk::bridge::multiscale::PartitionBoundaries const &)) &smtk::bridge::multiscale::PartitionBoundaries::operator=)
    .def_static("create", (std::shared_ptr<smtk::bridge::multiscale::PartitionBoundaries> (*)()) &smtk::bridge::multiscale::PartitionBoundaries::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::multiscale::PartitionBoundaries> (*)(::std::shared_ptr<smtk::bridge::multiscale::PartitionBoundaries> &)) &smtk::bridge::multiscale::PartitionBoundaries::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::bridge::multiscale::PartitionBoundaries> (smtk::bridge::multiscale::PartitionBoundaries::*)() const) &smtk::bridge::multiscale::PartitionBoundaries::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::bridge::multiscale::PartitionBoundaries> (smtk::bridge::multiscale::PartitionBoundaries::*)()) &smtk::bridge::multiscale::PartitionBoundaries::shared_from_this)
    ;
  return instance;
}

#endif
