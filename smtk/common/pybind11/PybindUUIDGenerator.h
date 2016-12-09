//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_common_UUIDGenerator_h
#define pybind_smtk_common_UUIDGenerator_h

#include <pybind11/pybind11.h>

#include "smtk/common/UUIDGenerator.h"

#include "smtk/common/UUID.h"

namespace py = pybind11;

py::class_< smtk::common::UUIDGenerator > pybind11_init_smtk_common_UUIDGenerator(py::module &m)
{
  py::class_< smtk::common::UUIDGenerator > instance(m, "UUIDGenerator");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::common::UUIDGenerator const &>())
    .def("deepcopy", (smtk::common::UUIDGenerator & (smtk::common::UUIDGenerator::*)(::smtk::common::UUIDGenerator const &)) &smtk::common::UUIDGenerator::operator=)
    .def("random", &smtk::common::UUIDGenerator::random)
    .def("null", &smtk::common::UUIDGenerator::null)
    ;
  return instance;
}

#endif
