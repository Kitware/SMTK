//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_operation_CreateResource_h
#define pybind_smtk_operation_CreateResource_h

#include <pybind11/pybind11.h>

#include "smtk/operation/CreateResource.h"

#include "smtk/operation/XMLOperator.h"

namespace py = pybind11;

PySharedPtrClass< smtk::operation::CreateResource, smtk::operation::XMLOperator > pybind11_init_smtk_operation_CreateResource(py::module &m)
{
  PySharedPtrClass< smtk::operation::CreateResource, smtk::operation::XMLOperator > instance(m, "CreateResource");
  instance
    .def(py::init<::smtk::operation::CreateResource const &>())
    .def("deepcopy", (smtk::operation::CreateResource & (smtk::operation::CreateResource::*)(::smtk::operation::CreateResource const &)) &smtk::operation::CreateResource::operator=)
    .def("classname", &smtk::operation::CreateResource::classname)
    .def_static("create", (std::shared_ptr<smtk::operation::CreateResource> (*)()) &smtk::operation::CreateResource::create)
    .def_static("create", (std::shared_ptr<smtk::operation::CreateResource> (*)(::std::shared_ptr<smtk::operation::CreateResource> &)) &smtk::operation::CreateResource::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::operation::CreateResource> (smtk::operation::CreateResource::*)() const) &smtk::operation::CreateResource::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::operation::CreateResource> (smtk::operation::CreateResource::*)()) &smtk::operation::CreateResource::shared_from_this)
    ;
  return instance;
}

#endif
