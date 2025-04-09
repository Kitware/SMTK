//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_operation_WriteResource_h
#define pybind_smtk_operation_WriteResource_h

#include <pybind11/pybind11.h>

#include "smtk/operation/operators/WriteResource.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::operation::WriteResource, smtk::operation::XMLOperation > pybind11_init_smtk_operation_WriteResource(py::module &m)
{
  PySharedPtrClass< smtk::operation::WriteResource, smtk::operation::XMLOperation > instance(m, "WriteResource");
  instance
    .def_static("create", (std::shared_ptr<smtk::operation::WriteResource> (*)()) &smtk::operation::WriteResource::create)
    .def_static("create", (std::shared_ptr<smtk::operation::WriteResource> (*)(::std::shared_ptr<smtk::operation::WriteResource> &)) &smtk::operation::WriteResource::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::operation::WriteResource> (smtk::operation::WriteResource::*)() const) &smtk::operation::WriteResource::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::operation::WriteResource> (smtk::operation::WriteResource::*)()) &smtk::operation::WriteResource::shared_from_this)
    ;
  return instance;
}

#endif
