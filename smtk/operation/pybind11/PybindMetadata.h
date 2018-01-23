//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_operation_Metadata_h
#define pybind_smtk_operation_Metadata_h

#include <pybind11/pybind11.h>

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Collection.h"

#include "smtk/operation/Metadata.h"

#include "smtk/resource/Component.h"

namespace py = pybind11;

py::class_< smtk::operation::Metadata > pybind11_init_smtk_operation_Metadata(py::module &m)
{
  py::class_< smtk::operation::Metadata > instance(m, "Metadata");
  instance
    .def(py::init<::smtk::operation::Metadata const &>())
    .def(py::init<::std::string const &, ::smtk::operation::NewOp::Index, ::smtk::operation::NewOp::Specification, ::std::function<std::shared_ptr<smtk::operation::NewOp> ()> >())
    .def("deepcopy", (smtk::operation::Metadata & (smtk::operation::Metadata::*)(::smtk::operation::Metadata const &)) &smtk::operation::Metadata::operator=)
    .def("acceptsComponent", &smtk::operation::Metadata::acceptsComponent, py::arg("c"))
    .def("index", &smtk::operation::Metadata::index)
    .def("specification", &smtk::operation::Metadata::specification)
    .def("uniqueName", &smtk::operation::Metadata::uniqueName)
    .def_readwrite("create", &smtk::operation::Metadata::create)
    ;
  return instance;
}

#endif
