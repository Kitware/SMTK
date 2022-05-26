//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_extension_paraview_appcomponents_pqSMTKResourcePanel_h
#define pybind_smtk_extension_paraview_appcomponents_pqSMTKResourcePanel_h

#include <pybind11/pybind11.h>

#include "smtk/extension/paraview/appcomponents/pqSMTKResourcePanel.h"

namespace py = pybind11;

inline py::class_< pqSMTKResourcePanel, QWidget > pybind11_init_pqSMTKResourcePanel(py::module &m)
{
  py::class_< pqSMTKResourcePanel, QWidget > instance(m, "pqSMTKResourcePanel");
  instance
    .def(py::init<::QWidget *>())
    .def("resourceBrowser", &pqSMTKResourcePanel::resourceBrowser)
    .def("setView", &pqSMTKResourcePanel::setView, py::arg("view"))
    ;
  return instance;
}

#endif
