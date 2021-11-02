//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_extension_paraview_appcomponents_pqSMTKResourceBrowser_h
#define pybind_smtk_extension_paraview_appcomponents_pqSMTKResourceBrowser_h

#include <pybind11/pybind11.h>

#include "smtk/extension/paraview/appcomponents/pqSMTKResourceBrowser.h"

namespace py = pybind11;

inline py::class_< pqSMTKResourceBrowser, smtk::extension::qtResourceBrowser > pybind11_init_pqSMTKResourceBrowser(py::module &m)
{
  py::class_< pqSMTKResourceBrowser, smtk::extension::qtResourceBrowser > instance(m, "pqSMTKResourceBrowser");
  instance
    .def(py::init<::smtk::view::Information const &>())
    .def_static("createViewWidget", &pqSMTKResourceBrowser::createViewWidget, py::arg("info"))
    .def_static("getJSONConfiguration", &pqSMTKResourceBrowser::getJSONConfiguration)
    .def("typeName", &pqSMTKResourceBrowser::typeName)
    .def_readonly_static("type_name", &pqSMTKResourceBrowser::type_name)
    ;
  return instance;
}

#endif
