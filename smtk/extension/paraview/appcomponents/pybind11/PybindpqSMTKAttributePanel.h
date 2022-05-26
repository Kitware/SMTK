//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_extension_paraview_appcomponents_pqSMTKAttributePanel_h
#define pybind_smtk_extension_paraview_appcomponents_pqSMTKAttributePanel_h

#include <pybind11/pybind11.h>

#include "smtk/extension/paraview/appcomponents/pqSMTKAttributePanel.h"

namespace py = pybind11;

inline py::class_< pqSMTKAttributePanel, QWidget > pybind11_init_pqSMTKAttributePanel(py::module &m)
{
  py::class_< pqSMTKAttributePanel, QWidget > instance(m, "pqSMTKAttributePanel");
  instance
    .def(py::init<::QWidget *>())
    .def("attributeUIManager", &pqSMTKAttributePanel::attributeUIManager)
    .def("displayPipelineSource", &pqSMTKAttributePanel::displayPipelineSource, py::arg("psrc"))
    .def("displayResource", &pqSMTKAttributePanel::displayResource, py::arg("rsrc"), py::arg("view") = nullptr, py::arg("advancedlevel") = 0)
    .def("displayResourceOnServer", &pqSMTKAttributePanel::displayResourceOnServer, py::arg("rsrc"), py::arg("view") = nullptr, py::arg("advancedlevel") = 0)
    .def("displayView", &pqSMTKAttributePanel::displayView, py::arg("view"))
    .def("resetPanel", &pqSMTKAttributePanel::resetPanel, py::arg("rsrcMgr"))
    .def("updatePipeline", &pqSMTKAttributePanel::updatePipeline)
    ;
  return instance;
}

#endif
