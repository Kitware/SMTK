//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_operators_Delete_h
#define pybind_smtk_model_operators_Delete_h

#include <pybind11/pybind11.h>

#include "smtk/model/operators/Delete.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::model::Delete, smtk::operation::XMLOperation > pybind11_init_smtk_model_Delete(py::module &m)
{
  PySharedPtrClass< smtk::model::Delete, smtk::operation::XMLOperation > instance(m, "Delete");
  instance
    .def(py::init<>())
    .def_static("create", (std::shared_ptr<smtk::model::Delete> (*)()) &smtk::model::Delete::create)
    .def_static("create", (std::shared_ptr<smtk::model::Delete> (*)(::std::shared_ptr<smtk::model::Delete> &)) &smtk::model::Delete::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::model::Delete> (smtk::model::Delete::*)() const) &smtk::model::Delete::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::model::Delete> (smtk::model::Delete::*)()) &smtk::model::Delete::shared_from_this)
    ;
  return instance;
}

#endif
