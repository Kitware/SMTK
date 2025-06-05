//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_operation_XMLOperation_h
#define pybind_smtk_operation_XMLOperation_h

#include <pybind11/pybind11.h>

#include "smtk/operation/XMLOperation.h"

#include "smtk/operation/Operation.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::operation::XMLOperation, smtk::operation::Operation > pybind11_init_smtk_operation_XMLOperation(py::module &m)
{
  PySharedPtrClass< smtk::operation::XMLOperation, smtk::operation::Operation > instance(m, "XMLOperation");
  instance
    .def("shared_from_this", (std::shared_ptr<const smtk::operation::XMLOperation> (smtk::operation::XMLOperation::*)() const) &smtk::operation::XMLOperation::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::operation::XMLOperation> (smtk::operation::XMLOperation::*)()) &smtk::operation::XMLOperation::shared_from_this)
    ;
  return instance;
}

#endif
