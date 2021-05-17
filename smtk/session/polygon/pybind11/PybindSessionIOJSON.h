//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_session_polygon_SessionIOJSON_h
#define pybind_smtk_session_polygon_SessionIOJSON_h

#include <pybind11/pybind11.h>

#include "smtk/session/polygon/SessionIOJSON.h"

#include "smtk/model/SessionIOJSON.h"

namespace py = pybind11;

inline py::class_< smtk::session::polygon::SessionIOJSON, smtk::model::SessionIOJSON > pybind11_init_smtk_session_polygon_SessionIOJSON(py::module &m)
{
  py::class_< smtk::session::polygon::SessionIOJSON, smtk::model::SessionIOJSON > instance(m, "SessionIOJSON");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::session::polygon::SessionIOJSON const &>())
    .def("deepcopy", (smtk::session::polygon::SessionIOJSON & (smtk::session::polygon::SessionIOJSON::*)(::smtk::session::polygon::SessionIOJSON const &)) &smtk::session::polygon::SessionIOJSON::operator=)
    .def_static("create", (std::shared_ptr<smtk::session::polygon::SessionIOJSON> (*)()) &smtk::session::polygon::SessionIOJSON::create)
    .def_static("create", (std::shared_ptr<smtk::session::polygon::SessionIOJSON> (*)(::std::shared_ptr<smtk::session::polygon::SessionIOJSON> &)) &smtk::session::polygon::SessionIOJSON::create, py::arg("ref"))
    ;
  return instance;
}

#endif
