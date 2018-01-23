//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_operation_MetadataContainer_h
#define pybind_smtk_operation_MetadataContainer_h

#include <pybind11/pybind11.h>

#include "smtk/operation/MetadataContainer.h"

namespace py = pybind11;

py::class_< smtk::operation::IndexTag > pybind11_init_smtk_operation_IndexTag(py::module &m)
{
  py::class_< smtk::operation::IndexTag > instance(m, "IndexTag");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::operation::IndexTag const &>())
    .def("deepcopy", (smtk::operation::IndexTag & (smtk::operation::IndexTag::*)(::smtk::operation::IndexTag const &)) &smtk::operation::IndexTag::operator=)
    ;
  return instance;
}

py::class_< smtk::operation::NameTag > pybind11_init_smtk_operation_NameTag(py::module &m)
{
  py::class_< smtk::operation::NameTag > instance(m, "NameTag");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::operation::NameTag const &>())
    .def("deepcopy", (smtk::operation::NameTag & (smtk::operation::NameTag::*)(::smtk::operation::NameTag const &)) &smtk::operation::NameTag::operator=)
    ;
  return instance;
}

#endif
