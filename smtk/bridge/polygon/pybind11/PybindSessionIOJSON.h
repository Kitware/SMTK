//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_bridge_polygon_SessionIOJSON_h
#define pybind_smtk_bridge_polygon_SessionIOJSON_h

#include <pybind11/pybind11.h>

#include "smtk/bridge/polygon/SessionIOJSON.h"

#include "smtk/model/Manager.h"
#include "smtk/model/SessionIOJSON.h"

#include "cJSON.h"

namespace py = pybind11;

py::class_< smtk::bridge::polygon::SessionIOJSON, smtk::model::SessionIOJSON > pybind11_init_smtk_bridge_polygon_SessionIOJSON(py::module &m)
{
  py::class_< smtk::bridge::polygon::SessionIOJSON, smtk::model::SessionIOJSON > instance(m, "SessionIOJSON");
  instance
    .def(py::init<>())
    .def(py::init<::smtk::bridge::polygon::SessionIOJSON const &>())
    .def("deepcopy", (smtk::bridge::polygon::SessionIOJSON & (smtk::bridge::polygon::SessionIOJSON::*)(::smtk::bridge::polygon::SessionIOJSON const &)) &smtk::bridge::polygon::SessionIOJSON::operator=)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::SessionIOJSON> (*)()) &smtk::bridge::polygon::SessionIOJSON::create)
    .def_static("create", (std::shared_ptr<smtk::bridge::polygon::SessionIOJSON> (*)(::std::shared_ptr<smtk::bridge::polygon::SessionIOJSON> &)) &smtk::bridge::polygon::SessionIOJSON::create, py::arg("ref"))
    ;
  return instance;
}

#endif
