//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pybind_smtk_extension_paraview_appcomponents_pqSMTKResourceRepresentation_h
#define pybind_smtk_extension_paraview_appcomponents_pqSMTKResourceRepresentation_h

#include <pybind11/pybind11.h>

#include "smtk/extension/paraview/appcomponents/pqSMTKResourceRepresentation.h"

namespace py = pybind11;

inline py::class_< pqSMTKResourceRepresentation, pqPipelineRepresentation > pybind11_init_pqSMTKResourceRepresentation(py::module &m)
{
  py::class_< pqSMTKResourceRepresentation, pqPipelineRepresentation > instance(m, "pqSMTKResourceRepresentation");
  instance
    .def(py::init<::QString const &, ::QString const &, ::vtkSMProxy *, ::pqServer *, ::QObject *>())
    .def("componentVisibilityChanged", &pqSMTKResourceRepresentation::componentVisibilityChanged, py::arg("comp"), py::arg("visible"))
    .def("onInputChanged", &pqSMTKResourceRepresentation::onInputChanged)
    .def("setVisibility", &pqSMTKResourceRepresentation::setVisibility, py::arg("comp"), py::arg("visible"))
    ;
  return instance;
}

#endif
