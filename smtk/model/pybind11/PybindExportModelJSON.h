//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_model_operators_ExportModelJSON_h
#define pybind_smtk_model_operators_ExportModelJSON_h

#include <pybind11/pybind11.h>

#include "smtk/model/operators/ExportModelJSON.h"

#include "smtk/operation/XMLOperation.h"

namespace py = pybind11;

inline PySharedPtrClass< smtk::model::ExportModelJSON, smtk::operation::XMLOperation > pybind11_init_smtk_model_ExportModelJSON(py::module &m)
{
  PySharedPtrClass< smtk::model::ExportModelJSON, smtk::operation::XMLOperation > instance(m, "ExportModelJSON");
  instance
    .def(py::init<>())
    .def_static("create", (std::shared_ptr<smtk::model::ExportModelJSON> (*)()) &smtk::model::ExportModelJSON::create)
    .def_static("create", (std::shared_ptr<smtk::model::ExportModelJSON> (*)(::std::shared_ptr<smtk::model::ExportModelJSON> &)) &smtk::model::ExportModelJSON::create, py::arg("ref"))
    .def("shared_from_this", (std::shared_ptr<const smtk::model::ExportModelJSON> (smtk::model::ExportModelJSON::*)() const) &smtk::model::ExportModelJSON::shared_from_this)
    .def("shared_from_this", (std::shared_ptr<smtk::model::ExportModelJSON> (smtk::model::ExportModelJSON::*)()) &smtk::model::ExportModelJSON::shared_from_this)
    ;
  return instance;
}

#endif
