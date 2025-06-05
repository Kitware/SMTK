//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_project_Operation_h
#define pybind_smtk_project_Operation_h

#include <pybind11/pybind11.h>

#include "smtk/project/Operation.h"

#include "smtk/operation/XMLOperation.h"
#include "smtk/project/Manager.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::project::Operation, smtk::operation::XMLOperation > pybind11_init_smtk_project_Operation(py::module &m)
{
  py::module::import("smtk.operation");

  PySharedPtrClass< smtk::project::Operation, smtk::operation::XMLOperation > instance(m, "Operation");
  instance
    .def("typeName", &smtk::project::Operation::typeName)
    .def("shared_from_this", (std::shared_ptr<smtk::project::Operation> (smtk::project::Operation::*)()) &smtk::project::Operation::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<const smtk::project::Operation> (smtk::project::Operation::*)() const) &smtk::project::Operation::shared_from_this)
    .def("setProjectManager", &smtk::project::Operation::setProjectManager, py::arg("arg0"))
    .def("projectManager", &smtk::project::Operation::projectManager)
    ;
  return instance;
}

#endif
