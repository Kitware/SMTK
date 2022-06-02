//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_extension_paraview_appcomponents_pqSMTKOperationPanel_h
#define pybind_smtk_extension_paraview_appcomponents_pqSMTKOperationPanel_h

#include <pybind11/pybind11.h>

#include "smtk/extension/paraview/appcomponents/pqSMTKOperationPanel.h"

namespace py = pybind11;

inline py::class_< pqSMTKOperationPanel, QWidget > pybind11_init_pqSMTKOperationPanel(py::module &m)
{
  py::class_< pqSMTKOperationPanel, QWidget > instance(m, "pqSMTKOperationPanel");
  instance
    .def(py::init<::QWidget *>())
    .def("attributeUIManager", &pqSMTKOperationPanel::attributeUIManager)
    .def("availableOperations", &pqSMTKOperationPanel::availableOperations)
    .def("cancelEditing", &pqSMTKOperationPanel::cancelEditing)
    .def("editOperation", (bool (pqSMTKOperationPanel::*)(::smtk::operation::Operation::Index)) &pqSMTKOperationPanel::editOperation, py::arg("index"))
    .def("editOperation", (bool (pqSMTKOperationPanel::*)(::smtk::operation::OperationPtr)) &pqSMTKOperationPanel::editOperation, py::arg("operation"))
    .def("observeWrapper", &pqSMTKOperationPanel::observeWrapper, py::arg("arg0"), py::arg("arg1"))
    .def("runOperation", (void (pqSMTKOperationPanel::*)()) &pqSMTKOperationPanel::runOperation)
    .def("runOperation", (void (pqSMTKOperationPanel::*)(::smtk::operation::OperationPtr)) &pqSMTKOperationPanel::runOperation, py::arg("operation"))
    .def("unobserveWrapper", &pqSMTKOperationPanel::unobserveWrapper, py::arg("arg0"), py::arg("arg1"))
    ;
  return instance;
}

#endif
