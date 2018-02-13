//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_operation_LoadResource_h
#define pybind_smtk_operation_LoadResource_h

#include <pybind11/pybind11.h>

#include "smtk/operation/LoadResource.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

PySharedPtrClass< smtk::operation::LoadResource, smtk::operation::XMLOperation > pybind11_init_smtk_operation_LoadResource(py::module &m)
{
  PySharedPtrClass< smtk::operation::LoadResource, smtk::operation::XMLOperation > instance(m, "LoadResource");
  instance
    .def(py::init<::smtk::operation::LoadResource const &>())
    .def("deepcopy", (smtk::operation::LoadResource & (smtk::operation::LoadResource::*)(::smtk::operation::LoadResource const &)) &smtk::operation::LoadResource::operator=)
    .def("classname", &smtk::operation::LoadResource::classname)
    .def_static("create", (std::shared_ptr<smtk::operation::LoadResource> (*)()) &smtk::operation::LoadResource::create)
    .def_static("create", (std::shared_ptr<smtk::operation::LoadResource> (*)(::std::shared_ptr<smtk::operation::LoadResource> &)) &smtk::operation::LoadResource::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::operation::LoadResource> (smtk::operation::LoadResource::*)() const) &smtk::operation::LoadResource::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::operation::LoadResource> (smtk::operation::LoadResource::*)()) &smtk::operation::LoadResource::shared_from_this)
    ;
  return instance;
}

#endif
