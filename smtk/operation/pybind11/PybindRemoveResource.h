//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_operation_RemoveResource_h
#define pybind_smtk_operation_RemoveResource_h

#include <pybind11/pybind11.h>

#include "smtk/operation/operators/RemoveResource.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::operation::RemoveResource, smtk::operation::XMLOperation > pybind11_init_smtk_operation_RemoveResource(py::module &m)
{
  PySharedPtrClass< smtk::operation::RemoveResource, smtk::operation::XMLOperation > instance(m, "RemoveResource");
  instance
    .def_static("create", (std::shared_ptr<smtk::operation::RemoveResource> (*)()) &smtk::operation::RemoveResource::create)
    .def_static("create", (std::shared_ptr<smtk::operation::RemoveResource> (*)(::std::shared_ptr<smtk::operation::RemoveResource> &)) &smtk::operation::RemoveResource::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::operation::RemoveResource> (smtk::operation::RemoveResource::*)() const) &smtk::operation::RemoveResource::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::operation::RemoveResource> (smtk::operation::RemoveResource::*)()) &smtk::operation::RemoveResource::shared_from_this)
    ;
  return instance;
}

#endif
