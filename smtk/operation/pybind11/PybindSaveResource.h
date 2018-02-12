//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_operation_SaveResource_h
#define pybind_smtk_operation_SaveResource_h

#include <pybind11/pybind11.h>

#include "smtk/operation/SaveResource.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

PySharedPtrClass< smtk::operation::SaveResource, smtk::operation::XMLOperation > pybind11_init_smtk_operation_SaveResource(py::module &m)
{
  PySharedPtrClass< smtk::operation::SaveResource, smtk::operation::XMLOperation > instance(m, "SaveResource");
  instance
    .def(py::init<::smtk::operation::SaveResource const &>())
    .def("deepcopy", (smtk::operation::SaveResource & (smtk::operation::SaveResource::*)(::smtk::operation::SaveResource const &)) &smtk::operation::SaveResource::operator=)
    .def("classname", &smtk::operation::SaveResource::classname)
    .def_static("create", (std::shared_ptr<smtk::operation::SaveResource> (*)()) &smtk::operation::SaveResource::create)
    .def_static("create", (std::shared_ptr<smtk::operation::SaveResource> (*)(::std::shared_ptr<smtk::operation::SaveResource> &)) &smtk::operation::SaveResource::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::operation::SaveResource> (smtk::operation::SaveResource::*)() const) &smtk::operation::SaveResource::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::operation::SaveResource> (smtk::operation::SaveResource::*)()) &smtk::operation::SaveResource::shared_from_this)
    ;
  return instance;
}

#endif
