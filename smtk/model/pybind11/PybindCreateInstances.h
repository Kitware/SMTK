//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_operators_CreateInstances_h
#define pybind_smtk_model_operators_CreateInstances_h

#include <pybind11/pybind11.h>

#include "smtk/model/operators/CreateInstances.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::model::CreateInstances, smtk::operation::XMLOperation > pybind11_init_smtk_model_CreateInstances(py::module &m)
{
  PySharedPtrClass< smtk::model::CreateInstances, smtk::operation::XMLOperation > instance(m, "CreateInstances");
  instance
    .def(py::init<>())
    .def_static("create", (std::shared_ptr<smtk::model::CreateInstances> (*)()) &smtk::model::CreateInstances::create)
    .def_static("create", (std::shared_ptr<smtk::model::CreateInstances> (*)(::std::shared_ptr<smtk::model::CreateInstances> &)) &smtk::model::CreateInstances::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::model::CreateInstances> (smtk::model::CreateInstances::*)() const) &smtk::model::CreateInstances::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::model::CreateInstances> (smtk::model::CreateInstances::*)()) &smtk::model::CreateInstances::shared_from_this)
    ;
  return instance;
}

#endif
