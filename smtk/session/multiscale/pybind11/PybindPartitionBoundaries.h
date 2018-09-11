//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_multiscale_operators_PartitionBoundaries_h
#define pybind_smtk_session_multiscale_operators_PartitionBoundaries_h

#include <pybind11/pybind11.h>

#include "smtk/session/multiscale/operators/PartitionBoundaries.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

PySharedPtrClass< smtk::session::multiscale::PartitionBoundaries, smtk::operation::XMLOperation > pybind11_init_smtk_session_multiscale_PartitionBoundaries(py::module &m)
{
  PySharedPtrClass< smtk::session::multiscale::PartitionBoundaries, smtk::operation::XMLOperation > instance(m, "PartitionBoundaries");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::session::multiscale::PartitionBoundaries const &>())
    .def("deepcopy", (smtk::session::multiscale::PartitionBoundaries & (smtk::session::multiscale::PartitionBoundaries::*)(::smtk::session::multiscale::PartitionBoundaries const &)) &smtk::session::multiscale::PartitionBoundaries::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::multiscale::PartitionBoundaries> (*)()) &smtk::session::multiscale::PartitionBoundaries::create)
    .def_static("create", (std::shared_ptr<smtk::session::multiscale::PartitionBoundaries> (*)(::std::shared_ptr<smtk::session::multiscale::PartitionBoundaries> &)) &smtk::session::multiscale::PartitionBoundaries::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::session::multiscale::PartitionBoundaries> (smtk::session::multiscale::PartitionBoundaries::*)() const) &smtk::session::multiscale::PartitionBoundaries::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::session::multiscale::PartitionBoundaries> (smtk::session::multiscale::PartitionBoundaries::*)()) &smtk::session::multiscale::PartitionBoundaries::shared_from_this)
    ;
  return instance;
}

#endif
