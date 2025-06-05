//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_operation_ReadResource_h
#define pybind_smtk_operation_ReadResource_h

#include <pybind11/pybind11.h>

#include "smtk/operation/operators/ReadResource.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::operation::ReadResource, smtk::operation::XMLOperation > pybind11_init_smtk_operation_ReadResource(py::module &m)
{
  PySharedPtrClass< smtk::operation::ReadResource, smtk::operation::XMLOperation > instance(m, "ReadResource");
  instance
    .def_static("create", (std::shared_ptr<smtk::operation::ReadResource> (*)()) &smtk::operation::ReadResource::create)
    .def_static("create", (std::shared_ptr<smtk::operation::ReadResource> (*)(::std::shared_ptr<smtk::operation::ReadResource> &)) &smtk::operation::ReadResource::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::operation::ReadResource> (smtk::operation::ReadResource::*)() const) &smtk::operation::ReadResource::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::operation::ReadResource> (smtk::operation::ReadResource::*)()) &smtk::operation::ReadResource::shared_from_this)
    ;
  return instance;
}

#endif
