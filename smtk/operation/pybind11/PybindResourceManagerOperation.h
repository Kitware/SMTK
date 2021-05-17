//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_operation_ResourceManagerOperation_h
#define pybind_smtk_operation_ResourceManagerOperation_h

#include <pybind11/pybind11.h>

#include "smtk/operation/ResourceManagerOperation.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::operation::ResourceManagerOperation, smtk::operation::XMLOperation > pybind11_init_smtk_operation_ResourceManagerOperation(py::module &m)
{
  PySharedPtrClass< smtk::operation::ResourceManagerOperation, smtk::operation::XMLOperation > instance(m, "ResourceManagerOperation");
  instance
    .def("deepcopy", (smtk::operation::XMLOperation & (smtk::operation::XMLOperation::*)(::smtk::operation::XMLOperation const &)) &smtk::operation::XMLOperation::operator=)
    .def("shared_from_this", (std::shared_ptr<const smtk::operation::XMLOperation> (smtk::operation::XMLOperation::*)() const) &smtk::operation::XMLOperation::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::operation::XMLOperation> (smtk::operation::XMLOperation::*)()) &smtk::operation::XMLOperation::shared_from_this)
    .def("resourceManager", &smtk::operation::ResourceManagerOperation::resourceManager)
    ;
  return instance;
}

#endif
