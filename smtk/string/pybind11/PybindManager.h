//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_string_Manager_h
#define pybind_smtk_string_Manager_h

#include <pybind11/pybind11.h>

#include "smtk/string/Manager.h"

namespace py = pybind11;

inline py::class_<smtk::string::Manager, std::shared_ptr<smtk::string::Manager>>
pybind11_init_smtk_string_Manager(py::module &m)
{
  py::class_<smtk::string::Manager, std::shared_ptr<smtk::string::Manager>> instance(m, "Manager");
  instance
    .def_static("create", &smtk::string::Manager::create)
    .def("manage", &smtk::string::Manager::manage, py::arg("string"))
    .def("unmanage", &smtk::string::Manager::unmanage, py::arg("hash"))
    .def("value", &smtk::string::Manager::value, py::arg("hash"))
    .def("find", &smtk::string::Manager::find, py::arg("string"))
    ;
  return instance;
}

#endif
